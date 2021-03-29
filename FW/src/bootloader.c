#include <string.h>
#include "bootloader.h"
#include "canbus_volz.h"
#include "global.h"
#include "keeloq.h"
#include "outputs.h"
#include "pwmout.h"
#include "rs485.h"
#include "rs232.h"
#include "serial.h"

#define BL_RS485_BAUDRATE	      	115200L

uint8_t BL_SyncroPWM[8] = {0x90, 0x01, 0x12, 0xaa, 0x34, 0x55, 0x59, 0xca};
uint8_t BL_buffer[BL_PAGE_SIZE];

uint16_t BL_startGood = 0;
uint16_t BL_startBad = 0;

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
  OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_OFF);
  vTaskDelay(50);

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Disable PWM */
      PWMOUT_Disable();
      /**< Enable UART transmission */
      PWMOUT_EnableUART(BL_UART_BAUDRATE);
      /**< Connect servo again */
      OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_ON);
      for (i = 0; i < BL_PWM_RESYNCS; i++)
      {
        /**< Send sync packet (8 bytes) */
        SERIAL_Send(BL_SyncroPWM, 8);
        /**< Wait for answer from servo */
        if ((SERIAL_Receive(data, 2, 10) == true) && (data[0] == BL_ANS_SYNC1) && (data[1] == BL_ANS_SYNC2))
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
      RS485_DisableRxInt();
      RS485_SetBaudrate(BL_UART_BAUDRATE);
      OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_ON);
      for (i = 0; i < BL_PWM_RESYNCS; i++)
      {
        vTaskDelay(1);
        memset(data, 0, 2);
        /**< Send sync packet (8 bytes) */
        RS485_Send(BL_SyncroPWM, 8);
        /**< Wait for answer from servo */
        if ((RS485_Receive(data, 2, 15) == true) && (data[0] == BL_ANS_SYNC1) && (data[1] == BL_ANS_SYNC2))
          break;
      }
      if (i < BL_PWM_RESYNCS)
        return true;
      RS485_SetBaudrate(GLOBAL_Baudrate);
      RS485_EnableRxInt();
      return false;
    case CONN_MODE_CAN:
      /**< Connect servo again */
      OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_ON);
      for (i = 0; i < BL_PWM_RESYNCS; i++)
      {
        if (CANBUS_StartBL() == true)
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RESYNCS)
        return true;
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
      //KEELOQ_Reconfigure();
      SERIAL_Disable();
      PWMOUT_Enable();
      break;
    case CONN_MODE_RS485:
      RS485_SetBaudrate(GLOBAL_Baudrate);
      RS485_EnableRxInt();
      break;
    case CONN_MODE_CAN:
      CANBUS_GoToApp();
      break;
  }
  /**< Disconnect servo from power supply */
  OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_OFF);
}

/** \brief Check CRC for one page
 *
 * \param [in] page Page number
 * \param [in] crc Right CRC16 value
 * \return True if CRC check was successful
 *
 */
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
        SERIAL_Send(tx_data, 4);
        if ((SERIAL_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
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
      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        if (CANBUS_CheckCRC(page, crc) == true)
          break;
        vTaskDelay(1);
      }
      if (i < BL_PWM_RETRY)
        return true;
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
        SERIAL_Send(tx_data, 4);
        if ((SERIAL_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
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
  RS232_DisableRxInt();
  /**< Get binary stream (256 bytes) */
  if (RS232_Receive(BL_buffer, BL_PAGE_SIZE, 300) == false)
  {
    /**< Enable UART interrupt */
    RS232_EnableRxInt();
    return false;
  }
  /**< Enable UART interrupt */
  RS232_EnableRxInt();

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
        SERIAL_Send(tx_data, 2);
        vTaskDelay(1);
        counter = 0;
        while (counter < BL_PAGE_SIZE / 2)
        {
          SERIAL_Send(&BL_buffer[counter], 8);
          counter += 8;
        }
        if ((SERIAL_Receive(rx_data, 2, 100) == false))//)) || (data[0] != BL_ANS_OK) || (data[1] != BL_ANS_OK))
          continue;
        vTaskDelay(1);
        /**< Send second part of the page */
        if (secured == true)
          tx_data[0] = BL_FLASH_PAGE_SEC_H;
        else
          tx_data[0] = BL_FLASH_PAGE_H;
        tx_data[1] = (page << 1) + 1;
        SERIAL_Send(tx_data, 2);
        vTaskDelay(1);
        while (counter < BL_PAGE_SIZE)
        {
          SERIAL_Send(&BL_buffer[counter], 8);
          counter += 8;
        }
        if ((SERIAL_Receive(rx_data, 2, 100) == false))// || (data[0] != BL_ANS_OK) || (data[1] != BL_ANS_OK))
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
          //while (RS485_TxComplete() == false);
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
          //while (RS485_TxComplete() == false);
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
      i = 0;
      while (i++ < BL_PWM_RETRY)
      {
        vTaskDelay(1);
        counter = 0;
        while ((counter < BL_PAGE_SIZE) && (i++ < BL_PWM_RETRY))
        {
          if (CANBUS_WriteToBuff((uint8_t)counter, (uint32_t*)&BL_buffer[counter]) == false)
            continue;
          i = 0;
          counter += sizeof(uint32_t);
        }
        if (i >= BL_PWM_RETRY)
          break;
        vTaskDelay(1);
        if (secured == true)
        {
          if (CANBUS_WriteEncPage(page) == false)
            continue;
        } else
        {
          if (CANBUS_WritePage(page) == false)
            continue;
        }
        break;
      }
      if (i < BL_PWM_RETRY)
        res = true;
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
        SERIAL_Send(tx_data, 3);
        if ((SERIAL_Receive(rx_data, 2, 20) == true) && (rx_data[0] == BL_ANS_OK) && (rx_data[1] == BL_ANS_OK))
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
