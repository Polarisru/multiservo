#include "defines.h"
#include "crc16.h"
#include "flash_if.h"
#include "programm.h"
#include "uart.h"
#include "xtea.h"

#define LEDS_GPIO             (GPIOB)//(GPIOD)

#define LEDS_PIN_LED1         (GPIO_Pin_8)//(GPIO_Pin_13)

#define LED_BLINK_VALUE       (200000)

uint8_t RecBuffer[PAGE_SIZE + sizeof(uint16_t)];
/**< Synchronization packet with precalculated CRC */
const uint8_t sync_packet[PACKET_SIZE] = {PACKET_HEADER, CMD_SYNC, 0x12, 0xAA, 0x34, 0x55, 0xFA, 0x07};
const char xtea_key[XTEA_KEY_LEN + 1] = "FwUpdate25121978";

/** \brief Initialize hardware modules
 *
 * \return Nothing
 *
 */
void DeviceInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

	// PCLK1 = APB1 clock max.42
	// PCLK2 = APB2 clock max.84
  RCC_PCLK1Config(RCC_HCLK_Div4);
  /**< PCLK2 = HCLK/2 */
  RCC_PCLK2Config(RCC_HCLK_Div2);

//  GPIO_DeInit(GPIOA);
//  GPIO_DeInit(GPIOB);
//  GPIO_DeInit(GPIOC);
//  GPIO_DeInit(GPIOD);

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);

  GPIO_StructInit(&GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = LEDS_PIN_LED1;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LEDS_GPIO, &GPIO_InitStructure);
}

/** \brief Run application at APPLICATION_ADDRESS
 *
 * \return Nothing
 *
 */
void ExecuteApp(void)
{
  typedef  void (*pFunction)(void);
  pFunction Jump_To_Application;
  uint32_t JumpAddress;

  if (((*(__IO uint32_t*)APPLICATION_ADDRESS) & 0x2FFC0000 ) == 0x20000000)
  {
    /**< Fictive function to jump to main application */
    JumpAddress = *(__IO uint32_t*)(APPLICATION_ADDRESS + 4);
    Jump_To_Application = (pFunction) JumpAddress;
    /**< Initialize user application's Stack Pointer */
    __set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
    /**< Rebase the vector table base address */
    //SCB->VTOR = ((uint32_t)APPLICATION_ADDRESS & SCB_VTOR_TBLOFF_Msk);
    /**< Jump to main application */
    Jump_To_Application();
  }
}

/** \brief Main bootloader procedure
 */
int main(void)
{
  uint32_t flashdestination;
  uint16_t crc16;
  uint16_t i;
  uint16_t page;
  uint32_t Counter = 0;
  bool ledBlink = false;
  uint8_t packet[PACKET_SIZE] = {0};
  uint8_t key[XTEA_KEY_LEN];

  DeviceInit();

  /**< Switch LED on */
  GPIO_WriteBit(LEDS_GPIO, LEDS_PIN_LED1, Bit_SET);

  UART_Configuration();

  for (Counter = 0; Counter < START_DELAY; Counter++)
  {
    if (UART_HaveData())
    {
      for (i = 0; i < PACKET_SIZE - 1; i++)
        packet[i] = packet[i + 1];
      packet[PACKET_SIZE - 1] = UART_GetChar();
      /**< Check if it's a sync packet */
      if (memcmp(packet, sync_packet, PACKET_SIZE) == 0)
        break;
    }
  }
  if (Counter >= START_DELAY)
  {
    /**< Switch LED off */
    GPIO_WriteBit(LEDS_GPIO, LEDS_PIN_LED1, Bit_RESET);
    /**< Jump to main app */
    ExecuteApp();
  }

  /**< Enable flash writing module */
  FLASH_If_Init();

  for (;;)
  {
    Counter++;
    if (Counter > LED_BLINK_VALUE)
    {
      Counter = 0;
      if (ledBlink)
        GPIO_WriteBit(LEDS_GPIO, LEDS_PIN_LED1, Bit_RESET);
      else
        GPIO_WriteBit(LEDS_GPIO, LEDS_PIN_LED1, Bit_SET);
      ledBlink = !ledBlink;
    }

    if (UART_HaveData())
    {
      /**< Have UART data waiting for receiving */
      for (i = 0; i < PACKET_SIZE - 1; i++)
        packet[i] = packet[i + 1];
      packet[PACKET_SIZE - 1] = UART_GetChar();

      if (packet[PACKET_OFFS_HEAD] != PACKET_HEADER)
        continue;
      crc16 = CRC16_Calc(packet, PACKET_SIZE - sizeof(uint16_t));
      if ((packet[PACKET_OFFS_CRC] != (uint8_t)(crc16 >> 8)) || (packet[PACKET_OFFS_CRC + 1] != (uint8_t)crc16))
        continue;

      if (packet[PACKET_OFFS_CMD] == CMD_FILL_BUFF)
      {
        /**< Fill buffer for writing to flash memory */
        /**< Receive page data */
        for (i = 0; i < (PAGE_SIZE + sizeof(uint16_t)); i++)
          RecBuffer[i] = UART_GetChar();
        crc16 = CRC16_Calc(RecBuffer, PAGE_SIZE);
        if (((uint8_t)(crc16 >> 8) == RecBuffer[PAGE_SIZE]) && ((uint8_t)crc16 == RecBuffer[PAGE_SIZE + 1]))
        {
          memcpy(key, (uint8_t*)xtea_key, XTEA_KEY_LEN);
          XTEA_Decrypt(RecBuffer, key, PAGE_SIZE);
          UART_SendChar(ANSW_OK);
          UART_SendChar(ANSW_OK);
        } else
        {
          UART_SendChar(ANSW_ERROR);
          UART_SendChar(ANSW_ERROR);
        }
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_FLASH_PAGE)
      {
        /**< Write filled page to flash memory */
        page = ((uint16_t)packet[PACKET_OFFS_PAGE] << 8) + packet[PACKET_OFFS_PAGE + 1];
        flashdestination = (uint32_t)page * PAGE_SIZE + APPLICATION_ADDRESS;
        /**< Erase sector if needed */
        if (FLASH_If_IsSectorStart(flashdestination) == 1)
        {
          if (FLASH_If_EraseSector(flashdestination) == 1)
          {
            UART_SendChar(ANSW_ERROR);
            UART_SendChar(ANSW_ERROR);
            continue;
          }
        }
        if (FLASH_If_Write(&flashdestination, (uint32_t*)RecBuffer, (uint16_t)(PAGE_SIZE / 4)) == 0)
        {
          UART_SendChar(ANSW_OK);
          UART_SendChar(ANSW_OK);
        } else
        {
          UART_SendChar(ANSW_ERROR);
          UART_SendChar(ANSW_ERROR);
        }
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_CHECK_FLASH)
      {
        /**< Check CRC for one page */
        page = ((uint16_t)packet[PACKET_OFFS_PAGE] << 8) + packet[PACKET_OFFS_PAGE + 1];
        flashdestination = (uint32_t)page * PAGE_SIZE + APPLICATION_ADDRESS;
        crc16 = CRC16_Calc((uint8_t*)flashdestination, PAGE_SIZE);
        if ((packet[PACKET_OFFS_DATA] == (uint8_t)(crc16 >>8)) && (packet[PACKET_OFFS_DATA + 1] == (uint8_t)crc16))
        {
          UART_SendChar(ANSW_OK);
          UART_SendChar(ANSW_OK);
        } else
        {
          UART_SendChar(ANSW_ERROR);
          UART_SendChar(ANSW_ERROR);
        }
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_RUN_APP)
      {
        /**< Jump to main app */
        ExecuteApp();
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_SYNC)
      {
        /**< Synchronization packet */
        UART_SendChar('S');
        UART_SendChar('Y');
      }
    }
  }
}
