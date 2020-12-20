#include <string.h>
#include "bootloader.h"
#include "global.h"
#include "keeloq.h"
#include "outputs.h"
#include "rs485.h"
#include "rs232.h"

#define BL_RS485_BAUDRATE	      	115200L

//const char BL_str[] = "1234567890ABCDEFGHIJKLMNOPQRSTUVWXYZ\n";
uint8_t BL_SyncroPWM[8] = {0x90, 0x01, 0x12, 0xaa, 0x34, 0x55, 0x59, 0xca};
uint8_t BL_buffer[BL_PAGE_SIZE];

uint16_t BL_startGood = 0;
uint16_t BL_startBad = 0;

/** \brief RTOS task for processing bootloader functionality
 */
//void BOOTLOADER_Task(void *pParameters)
//{
//  uint8_t i;
//
//  while (1)
//  {
//    /**< Wait for notification from the RX interrupt */
//    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//
//    UART_DisableRxInt();
//
//    /**< Increase task priority to be the most important task */
//    vTaskPrioritySet(NULL, REALTIME_PRIORITY);
//
//    for (i = 0; i < 100; i++)
//      UART_SendData((char*)BL_str, strlen(BL_str));
//
//    UART_EnableRxInt();
//
//    /**< Return to normal priority */
//    vTaskPrioritySet(NULL, BL_TASK_PRIORITY);
//  }
//}

/** \brief Enable Bootloader UART
 *
 * \return Nothing
 *
 */
//static void BOOTLOADER_EnableUART(void)
//{
//  GPIO_InitTypeDef  GPIO_InitStructure;
//  USART_InitTypeDef  USART_InitStructure;
//
//  BL_UART_CLOCK_ENABLE;
//
//  /**< Configure USART Tx and Rx as alternate function */
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_Pin = BL_UART_TX_PIN | BL_UART_RX_PIN;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_Init(BL_UART_GPIO, &GPIO_InitStructure);
//
//  GPIO_PinAFConfig(BL_UART_GPIO, BL_UART_RX_SOURCE, UART_GPIO_AF);
//  GPIO_PinAFConfig(BL_UART_GPIO, BL_UART_TX_SOURCE, UART_GPIO_AF);
//
//  USART_InitStructure.USART_BaudRate = BL_UART_BAUDRATE;
//  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
//  USART_InitStructure.USART_StopBits = USART_StopBits_2;
//  USART_InitStructure.USART_Parity = USART_Parity_No;
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//
//  USART_Init(BL_UART_NUM, &USART_InitStructure);
//  USART_Cmd(BL_UART_NUM, ENABLE);
//}

/** \brief Disable Bootloader UART
 *
 * \return Nothing
 *
 */
//static void BOOTLOADER_DisableUART(void)
//{
//  USART_Cmd(BL_UART_NUM, DISABLE);
//}

/** \brief Send data to Bootloader UART
 *
 * \param [in] data Output data buffer for sending via UART
 * \param [in] len Length of data to send
 * \return Nothing
 *
 */
static void BOOTLOADER_Send(uint8_t *data, uint8_t len)
{
  //BL_UART_NUM->CR1 &= ~USART_Mode_Rx;
  //BL_UART_NUM->CR1 |= USART_Mode_Tx;
//  while (len--)
//  {
//    USART_SendData(BL_UART_NUM, *data++);
//    BL_UART_NUM->DR = *data++;
//    /**< Wait till sending is complete */
//    while ((BL_UART_NUM->SR & USART_SR_TC) == 0);
//  }
  KEELOQ_SendUART(data, len);
}

/** \brief Receive data from Bootloader UART
 *
 * \param [out] data Pointer to data buffer
 * \param [in] len Length of data to receive
 * \param [in] timeout Timeout to receive data in system ticks (1ms)
 * \return True if succeed
 *
 */
static bool BOOTLOADER_Receive(uint8_t *data, uint8_t len, uint16_t timeout)
{
  uint32_t ticks = xTaskGetTickCount();

  KEELOQ_ConfigureRxDMA();
  //BL_UART_NUM->CR1 |= USART_Mode_Rx;
  while (len--)
  {
    /**< Wait until the data is ready to be received */
    //while (((BL_UART_NUM->SR & USART_SR_RXNE) == 0) && ((xTaskGetTickCount() - ticks) < timeout));
    KEELOQ_DoReceiveUART();
    while (KEELOQ_WaitReceiveUART() && ((xTaskGetTickCount() - ticks) < timeout));
    /**< Read RX data, combine with DR mask */
    //*data++ = (uint8_t)(BL_UART_NUM->DR & 0xFF);
    *data++ = KEELOQ_GetReceiveUART();
  }

  KEELOQ_ReconfigureDMA();

  return true;
}

/** \brief Start bootloader, try to sync with servo
 *
 * \return True if succeed
 *
 */
bool BOOTLOADER_Start(void)
{
  uint8_t i;
  uint8_t data[2] = {0};

  /**< Disconnect servo from power supply */
  //RELAYS_SetCombination(RELAYS_CMB_OFF);
  vTaskDelay(10);

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Disable PWM */
      //PWM_DisableChannel(PWM_CH1);
      /**< Enable UART transmission */
      //KEELOQ_ConfigureForUART(BL_UART_BAUDRATE);
      //OUTPUTS_Switch(OUTPUTS_PWM_EN, OUTPUTS_SWITCH_ON);

      /**< Connect servo again */
      //RELAYS_SetCombination(RELAYS_CMB_WRK);
      for (i = 0; i < BL_PWM_RESYNCS; i++)
      {
        /**< Send sync packet (8 bytes) */
        BOOTLOADER_Send(BL_SyncroPWM, 8);
        /**< Wait for answer from servo */
        if ((BOOTLOADER_Receive(data, 2, 10) == true) && (data[0] == BL_ANS_SYNC1) && (data[1] == BL_ANS_SYNC2))
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RESYNCS)
      {
        return true;
      }
      return false;
    case CONN_MODE_RS485:
      /**< Connect servo again */
      //RELAYS_SetCombination(RELAYS_CMB_WRK);
      RS485_DisableRxInt();
      RS485_SetBaudrate(BL_UART_BAUDRATE);
      for (i = 0; i < BL_PWM_RESYNCS; i++)
      {
        /**< Send sync packet (8 bytes) */
        RS485_Send(BL_SyncroPWM, 8);
        /**< Wait for answer from servo */
        if ((RS485_Receive(data, 2, 10) == true) && (data[0] == BL_ANS_SYNC1) && (data[1] == BL_ANS_SYNC2))
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RESYNCS)
        return true;
      RS485_SetBaudrate(BL_RS485_BAUDRATE);
      RS485_EnableRxInt();
      return false;
    case CONN_MODE_CAN:

      return false;
    default:
      return false;
  }
}

/** \brief Stop bootloader mode, switch off servo
 *
 * \return Nothing
 *
 */
void BOOTLOADER_Stop(void)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      KEELOQ_Reconfigure();
      //PWM_EnableChannel(PWM_CH1);
      //OUTPUTS_Switch(OUTPUTS_PWM_EN, OUTPUTS_SWITCH_ON);
      break;
    case CONN_MODE_RS485:
      RS485_SetBaudrate(GLOBAL_Baudrate);
      RS485_EnableRxInt();
      break;
    case CONN_MODE_CAN:
      break;
  }
  /**< Disconnect servo from power supply */
  //RELAYS_SetCombination(RELAYS_CMB_OFF);
}

bool BOOTLOADER_CheckCRC(uint8_t page, uint16_t crc)
{
  uint8_t tx_data[4];
  uint8_t rx_data[2];
  uint8_t i;

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Build CRC verify packet (4 bytes) */
      tx_data[0] = BL_VERIFY_PAGE;
      tx_data[1] = page;
      tx_data[2] = (uint8_t)(crc >> 8);
      tx_data[3] = (uint8_t)crc;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        rx_data[0] = 0;
        rx_data[1] = 0;
        BOOTLOADER_Send(tx_data, 4);
        if ((BOOTLOADER_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
        vTaskDelay(2);
        BL_startBad++;
      }
      if (i < BL_PWM_RETRY)
      {
        BL_startGood++;
        return true;
      }
      return false;
    case CONN_MODE_RS485:
      /**< Build CRC verify packet (4 bytes) */
      tx_data[0] = BL_VERIFY_PAGE;
      tx_data[1] = page;
      tx_data[2] = (uint8_t)(crc >> 8);
      tx_data[3] = (uint8_t)crc;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        RS485_Send(tx_data, 4);
        if ((RS485_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RETRY)
        return true;
      return false;
    case CONN_MODE_CAN:
      return false;
    default:
      return false;
  }

  return false;
}

/** \brief Check EEPROM page
 *
 * \param [in] page Number of the page to check
 * \param [in] len Value of CRC16 to compare
 * \return True if succeed
 *
 */
bool BOOTLOADER_CheckEEPROM(uint8_t page, uint16_t crc)
{
  uint8_t tx_data[4];
  uint8_t rx_data[2];
  uint8_t i;

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Build EEPROM CRC verify packet (4 bytes) */
      tx_data[0] = BL_VERIFY_EEPROM;
      tx_data[1] = page;
      tx_data[2] = (uint8_t)(crc >> 8);
      tx_data[3] = (uint8_t)crc;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        BOOTLOADER_Send(tx_data, 4);
        if ((BOOTLOADER_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
      }
      if (i < BL_PWM_RETRY)
        return true;
      break;
    case CONN_MODE_RS485:
      /**< Build EEPROM CRC verify packet (4 bytes) */
      tx_data[0] = BL_VERIFY_EEPROM;
      tx_data[1] = page;
      tx_data[2] = (uint8_t)(crc >> 8);
      tx_data[3] = (uint8_t)crc;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        RS485_Send(tx_data, 4);
        if ((RS485_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RETRY)
        return true;
      break;
    case CONN_MODE_CAN:
      break;
  }

  return false;
}

/** \brief Write one page to Flash
 *
 * \param [in] page Number of the page to write
 * \return True if succeed
 *
 */
bool BOOTLOADER_WriteFlash(uint8_t page, bool secured)
{
  uint8_t tx_data[4];
  uint8_t rx_data[2];
  uint8_t i;
  bool res = false;
  uint16_t counter;

  /**< Should receive more data here, disable UART interrupt to get binary stream */
  UART_DisableRxInt();
  /**< Get binary stream (256 bytes) */
  if (UART_Receive(BL_buffer, BL_PAGE_SIZE, 300) == false)
  {
    /**< Enable UART interrupt */
    UART_EnableRxInt();
    return false;
  }
  /**< Enable UART interrupt */
  UART_EnableRxInt();

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        vTaskDelay(1);
        /**< Send first part of the page */
        if (secured == true)
          tx_data[0] = BL_FLASH_PAGE_SEC_L;
        else
          tx_data[0] = BL_FLASH_PAGE_L;
        tx_data[1] = page << 1;
        BOOTLOADER_Send(tx_data, 2);
        vTaskDelay(1);
        counter = 0;
        while (counter < BL_PAGE_SIZE / 2)
        {
          BOOTLOADER_Send(&BL_buffer[counter], 8);
          counter += 8;
        }
        if ((BOOTLOADER_Receive(rx_data, 2, 100) == false))//)) || (data[0] != BL_ANS_OK) || (data[1] != BL_ANS_OK))
          continue;
        vTaskDelay(1);
        /**< Send second part of the page */
        if (secured == true)
          tx_data[0] = BL_FLASH_PAGE_SEC_H;
        else
          tx_data[0] = BL_FLASH_PAGE_H;
        tx_data[1] = (page << 1) + 1;
        BOOTLOADER_Send(tx_data, 2);
        vTaskDelay(1);
        while (counter < BL_PAGE_SIZE)
        {
          BOOTLOADER_Send(&BL_buffer[counter], 8);
          counter += 8;
        }
        if ((BOOTLOADER_Receive(rx_data, 2, 100) == false))// || (data[0] != BL_ANS_OK) || (data[1] != BL_ANS_OK))
          continue;
        /**< Everything ok, go out */
        break;
      }
      if (i < BL_PWM_RETRY)
        res = true;
      break;
    case CONN_MODE_RS485:
      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        vTaskDelay(1);
        /**< Send first part of the page */
        if (secured == true)
          tx_data[0] = BL_FLASH_PAGE_SEC_L;
        else
          tx_data[0] = BL_FLASH_PAGE_L;
        tx_data[1] = page << 1;
        RS485_Send(tx_data, 2);
        vTaskDelay(1);
        counter = 0;
        while (counter < BL_PAGE_SIZE / 2)
        {
          RS485_Send(&BL_buffer[counter], 8);
          while (RS485_TxComplete() == false);
          counter += 8;
        }
        if ((RS485_Receive(rx_data, 2, 100) == false) || (rx_data[0] != BL_ANS_OK) || (rx_data[1] != BL_ANS_OK))
          continue;
        vTaskDelay(1);
        /**< Send second part of the page */
        if (secured == true)
          tx_data[0] = BL_FLASH_PAGE_SEC_H;
        else
          tx_data[0] = BL_FLASH_PAGE_H;
        tx_data[1] = (page << 1) + 1;
        RS485_Send(tx_data, 2);
        vTaskDelay(1);
        while (counter < BL_PAGE_SIZE)
        {
          RS485_Send(&BL_buffer[counter], 8);
          while (RS485_TxComplete() == false);
          counter += 8;
        }
        if ((RS485_Receive(rx_data, 2, 100) == false) || (rx_data[0] != BL_ANS_OK) || (rx_data[1] != BL_ANS_OK))
          continue;
        /**< Everything ok, go out */
        break;
      }
      if (i < BL_PWM_RETRY)
        res = true;
      break;
    case CONN_MODE_CAN:
      break;
  }

  return res;
}

/** \brief Write EEPROM value
 *
 * \param [in] addr Address in EEPROM to write
 * \param [in] val Value to write
 * \return True if succeed
 *
 */
bool BOOTLOADER_WriteEEPROM(uint16_t addr, uint8_t val)
{
  uint8_t i;
  uint8_t tx_data[3];
  uint8_t rx_data[2];

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Build packet (3 bytes) */
      if (addr >= 0x200)
        break;
      if (addr >= 0x100)
        tx_data[0] = BL_WR_EEPROM_H;
      else
        tx_data[0] = BL_WR_EEPROM_L;
      tx_data[1] = (uint8_t)addr;
      tx_data[2] = val;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        BOOTLOADER_Send(tx_data, 3);
        if ((BOOTLOADER_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
      }
      if (i < BL_PWM_RETRY)
        return true;
      break;
    case CONN_MODE_RS485:
      /**< Build packet (3 bytes) */
      if (addr >= 0x200)
        break;
      if (addr >= 0x100)
        tx_data[0] = BL_WR_EEPROM_H;
      else
        tx_data[0] = BL_WR_EEPROM_L;
      tx_data[1] = (uint8_t)addr;
      tx_data[2] = val;

      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        RS485_Send(tx_data, 3);
        if ((RS485_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
          break;
      }
      if (i < BL_PWM_RETRY)
        return true;
      break;
    case CONN_MODE_CAN:
      break;
  }

  return false;
}
