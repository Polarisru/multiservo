#include "driver_clk.h"
#include "driver_flash.h"
#include "driver_gpio.h"
#include "defines.h"
#include "crc16.h"
#include "programm.h"
#include "rs232.h"
#include "xtea.h"

#define LEDS_GPIO             (GPIO_PORTA)
#define LEDS_PIN_LED1         (2)

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
	CLK_Init();

  /**< Configure LED pin */
  GPIO_ClearPin(LEDS_GPIO, LEDS_PIN_LED1);
  GPIO_SetDir(LEDS_GPIO, LEDS_PIN_LED1, true);
  GPIO_SetFunction(LEDS_GPIO, LEDS_PIN_LED1, GPIO_PIN_FUNC_OFF);
  /**< Switch LED on */
  GPIO_SetPin(LEDS_GPIO, LEDS_PIN_LED1);
  /**< Configure UART */
  RS232_Configuration();
}

/**< Disable all important peripherals */
void DisableHardware(void)
{
  RS232_Disable();
}

/** \brief Jump to main application, disable all important peripherals
 *
 * \return Nothing
 *
 */
void JumpToApp(void)
{
  /**< Pointer to the Application Section  */
  void (*application_code_entry)(void);
  /**< Disable all important peripherals */
  DisableHardware();
  /**< Disable interrupts */
  //__disable_irq();
  /**< Rebase the Stack Pointer  */
  __set_MSP(*(uint32_t *)FLASH_APP_ADDRESS);
  /**< Rebase the vector table base address TODO: use RAM  */
  SCB->VTOR = ((uint32_t)FLASH_APP_ADDRESS & SCB_VTOR_TBLOFF_Msk);
  /**< Load the Reset Handler address of the application */
  application_code_entry = (void (*)(void))(uint32_t *)(*(uint32_t *)(FLASH_APP_ADDRESS + 4));
  /**< Jump to user Reset Handler in the application */
  application_code_entry();
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
  uint8_t packet[PACKET_SIZE] = {0};
  uint8_t key[XTEA_KEY_LEN];

  DeviceInit();

  for (Counter = 0; Counter < START_DELAY; Counter++)
  {
    if (RS232_HaveData())
    {
      for (i = 0; i < PACKET_SIZE - 1; i++)
        packet[i] = packet[i + 1];
      packet[PACKET_SIZE - 1] = RS232_GetChar();
      /**< Check if it's a sync packet */
      if (memcmp(packet, sync_packet, PACKET_SIZE) == 0)
        break;
    }
  }
  if (Counter >= START_DELAY)
  {
    /**< Switch LED off */
    GPIO_ClearPin(LEDS_GPIO, LEDS_PIN_LED1);
    /**< Jump to main app */
    JumpToApp();
  }

  for (;;)
  {
    Counter++;
    if (Counter > LED_BLINK_VALUE)
    {
      Counter = 0;
      GPIO_TogglePin(LEDS_GPIO, LEDS_PIN_LED1);
    }

    if (RS232_HaveData())
    {
      /**< Have UART data waiting for receiving */
      for (i = 0; i < PACKET_SIZE - 1; i++)
        packet[i] = packet[i + 1];
      packet[PACKET_SIZE - 1] = RS232_GetChar();

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
          RecBuffer[i] = RS232_GetChar();
        crc16 = CRC16_Calc(RecBuffer, PAGE_SIZE);
        if (((uint8_t)(crc16 >> 8) == RecBuffer[PAGE_SIZE]) && ((uint8_t)crc16 == RecBuffer[PAGE_SIZE + 1]))
        {
          memcpy(key, (uint8_t*)xtea_key, XTEA_KEY_LEN);
          XTEA_Decrypt(RecBuffer, key, PAGE_SIZE);
          RS232_SendChar(ANSW_OK);
          RS232_SendChar(ANSW_OK);
        } else
        {
          RS232_SendChar(ANSW_ERROR);
          RS232_SendChar(ANSW_ERROR);
        }
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_FLASH_PAGE)
      {
        /**< Write filled page to flash memory */
        page = ((uint16_t)packet[PACKET_OFFS_PAGE] << 8) + packet[PACKET_OFFS_PAGE + 1];
        flashdestination = (uint32_t)page * PAGE_SIZE + FLASH_APP_ADDRESS;
        /**< Erase sector if needed */
        FLASH_EraseRow((uint32_t*)flashdestination);
//        {
//          RS232_SendChar(ANSW_ERROR);
//          RS232_SendChar(ANSW_ERROR);
//          continue;
//        }
        FLASH_WriteWords((uint32_t*)flashdestination, (uint32_t*)RecBuffer, (uint16_t)(PAGE_SIZE / 4));
//        {
//          RS232_SendChar(ANSW_OK);
//          RS232_SendChar(ANSW_OK);
//        } else
//        {
//          RS232_SendChar(ANSW_ERROR);
//          RS232_SendChar(ANSW_ERROR);
//        }
        RS232_SendChar(ANSW_OK);
        RS232_SendChar(ANSW_OK);
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_CHECK_FLASH)
      {
        /**< Check CRC for one page */
        page = ((uint16_t)packet[PACKET_OFFS_PAGE] << 8) + packet[PACKET_OFFS_PAGE + 1];
        flashdestination = (uint32_t)page * PAGE_SIZE + FLASH_APP_ADDRESS;
        crc16 = CRC16_Calc((uint8_t*)flashdestination, PAGE_SIZE);
        if ((packet[PACKET_OFFS_DATA] == (uint8_t)(crc16 >>8)) && (packet[PACKET_OFFS_DATA + 1] == (uint8_t)crc16))
        {
          RS232_SendChar(ANSW_OK);
          RS232_SendChar(ANSW_OK);
        } else
        {
          RS232_SendChar(ANSW_ERROR);
          RS232_SendChar(ANSW_ERROR);
        }
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_RUN_APP)
      {
        /**< Jump to main app */
        JumpToApp();
      } else
      if (packet[PACKET_OFFS_CMD] == CMD_SYNC)
      {
        /**< Synchronization packet */
        RS232_SendChar('S');
        RS232_SendChar('Y');
      }
    }
  }
}
