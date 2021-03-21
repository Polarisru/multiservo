#include "canbus.h"
#include "canbus_volz.h"

static uint8_t tx_buff[CANBUS_MAX_LEN];
static uint8_t CANBUS_RxBuffer[CANBUS_MAX_LEN];
static uint8_t CANBUS_Id = CANMSG_ID_BROADCAST;

/** \brief Send CAN packet and wait for reply if needed
 *
 * \param [in] tx_data Data buffer with packet to send
 * \param [in] tx_len Length of data packet to send
 * \param [out] rx_data Buffer with reply data
 * \param [in] timeout Timeout value in ms
 * \return True if reply was received
 *
 */
bool CANBUS_Transfer(uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint8_t timeout)
{
  uint8_t errors = 0;
  uint32_t ticks;
  uint32_t id;
  TCanMsg rx_msg;

  /**< Build CAN packet here */
  id = CANBUS_DFLT_ID | CANBUS_Id;

  while (errors < CANBUS_MAX_RETRIES)
  {
    /**< First empty FIFO to avoid duplicated messages from last communication */
    if (CANBUS_SendMessage(id, tx_data, tx_len, false, false, false) == false)
      return false;
    /**< Wait till reply is received or timeout occurred */
    ticks = xTaskGetTickCount() + timeout;
    while (xTaskGetTickCount() < ticks)
    {
      if (CANBUS_ReceiveMessage(&rx_msg) == true)
      {
        if (rx_msg.data[CANMSG_OFFS_CMD] == (tx_data[CANMSG_OFFS_CMD] | CANMSG_CMD_REPLY_BIT))
        {
          memcpy(rx_data, rx_msg.data, rx_msg.len);
          return true;
        }
      }
      vTaskDelay(1);
    }

    errors++;
  }

  return false;
}

/** \brief Set Id for CAN bus
 *
 * \param [in] id Id to set
 * \return void
 *
 */
bool CANBUS_SetId(uint8_t id)
{
  if (id > CANMSG_ID_MASK)
    return false;
  CANBUS_Id = id & CANMSG_ID_MASK;

  return true;
}

/** \brief Set servo position
 *
 * \param [in] pos New position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_SetPosition(float pos)
{
  uint16_t val;

  if (pos < -SERVO_MAX_ANGLE)
    pos = -SERVO_MAX_ANGLE;
  if (pos > SERVO_MAX_ANGLE)
    pos = SERVO_MAX_ANGLE;

  val = (uint16_t)((int16_t)(pos * 10)) & 0xFFF;

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_SET_POS;
  tx_buff[CANMSG_OFFS_DATA] = (uint8_t)val;
  tx_buff[CANMSG_OFFS_DATA + 1] = (uint8_t)(val >> 8);

  if (CANBUS_Transfer(tx_buff, 3, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return true;
}

/** \brief Read current servo position
 *
 * \param [out] pos Pointer to position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_GetPosition(float *pos)
{
  uint16_t val;

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_GET_POS;

  if (CANBUS_Transfer(tx_buff, 1, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  val = ((uint16_t)CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1] << 8) + CANBUS_RxBuffer[CANMSG_OFFS_DATA];
  if (val >= 0x800)
    val |= 0xF000;
  *pos = (float)((int16_t)val) / 10;

  return true;
}

/** \brief Read byte from servo EEPROM
 *
 * \param [in] addr Address to read from
 * \param [out] value Pointer to store read value
 * \return True if succeed
 *
 */
bool CANBUS_ReadByte(uint16_t addr, uint8_t *value)
{
  if (addr >= 0x100)
  {
    return 0;
  }

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_READ_EE;
  tx_buff[CANMSG_OFFS_DATA] = (uint8_t)addr;

  if (CANBUS_Transfer(tx_buff, 2, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  *value = CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1];

  return true;
}

/** \brief Write byte to servo EEPROM
 *
 * \param [in] addr Address to write to
 * \param [in] value Value to write
 * \return True if succeed
 *
 */
bool CANBUS_WriteByte(uint16_t addr, uint8_t value)
{
  if (addr >= 0x100)
  {
    return true;
  }

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_WRITE_EE;
  tx_buff[CANMSG_OFFS_DATA] = (uint8_t)addr;
  tx_buff[CANMSG_OFFS_DATA + 1] = value;

  if (CANBUS_Transfer(tx_buff, 3, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return true;
}

/** \brief Read parameter from GetPos packet
 *
 * \param [in] offset Parameter offset in data packet
 * \param [out] value Pointer to value for readout
 * \return True if succeed
 *
 */
static bool CANBUS_ReadParameter(uint8_t offset, uint8_t *value)
{
  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_GET_POS;

  if (CANBUS_Transfer(tx_buff, 1, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  *value = CANBUS_RxBuffer[CANMSG_OFFS_DATA + offset];

  return true;
}

/** \brief Get current value from servo
 *
 * \param [out] value Pointer to value for current readout
 * \return True if succeed
 *
 */
bool CANBUS_GetCurrent(uint8_t *value)
{
  return CANBUS_ReadParameter(3, value);
}

/** \brief Get voltage value from servo
 *
 * \param [out] value Pointer to value for voltage readout
 * \return True if succeed
 *
 */
bool CANBUS_GetVoltage(uint8_t *value)
{
  return CANBUS_ReadParameter(4, value);
}

/** \brief Get temperature value from servo
 *
 * \param [out] value Pointer to value for temperature readout
 * \return True if succeed
 *
 */
bool CANBUS_GetTemperature(uint8_t *value)
{
  return CANBUS_ReadParameter(6, value);
}

/** \brief Set motor mode (normal\CW\CCW)
 *
 * \param [in] mode Motor mode to set
 * \return True if succeed
 *
 */
bool CANBUS_SetMotorMode(uint8_t mode)
{
  uint8_t len = 2;
  uint8_t new_mode;

  switch (mode)
  {
    case MOTOR_MOVE_NORMAL:
      new_mode = CANMSG_EXECUTE_NORMMODE;
      break;
    case MOTOR_MOVE_CW:
      new_mode = CANMSG_EXECUTE_ROTCW;
      break;
    case MOTOR_MOVE_CCW:
      new_mode = CANMSG_EXECUTE_ROTCCW;
      break;
    case MOTOR_MOVE_FREE:
      new_mode = CANMSG_EXECUTE_FREE;
      break;
    default:
      return false;
  }

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_EXECUTE;
  tx_buff[CANMSG_OFFS_DATA] = new_mode;
  if ((new_mode == CANMSG_EXECUTE_ROTCW) || (new_mode == CANMSG_EXECUTE_ROTCCW))
  {
    tx_buff[CANMSG_OFFS_DATA + 1] = 0xFF;
    len++;
  }

  if (CANBUS_Transfer(tx_buff, len, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return (CANBUS_RxBuffer[CANMSG_OFFS_DATA] == CANMSG_ANSWER_OK_SIGN);
}

/** \brief Just send bootloader starting command
 *
 * \return True if succeed
 *
 */
bool CANBUS_StartBL(void)
{
  uint32_t uval32;

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_BOOTLOADER;
  tx_buff[CANMSG_OFFS_DATA] = FLASH_CMD_STARTBL;
  uval32 = CANMSG_SIGNATURE_START;
  memcpy(&tx_buff[CANMSG_OFFS_DATA + 1], &uval32, sizeof(uint32_t));
  if (CANBUS_Transfer(tx_buff, sizeof(uint8_t) * 2 + sizeof(uint32_t), CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return (CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1] == CANMSG_ANSWER_OK_SIGN);
}

/** \brief Go to application
 *
 * \return True if succeed
 *
 */
bool CANBUS_GoToApp(void)
{
  uint32_t uval32;

  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_BOOTLOADER;
  tx_buff[CANMSG_OFFS_DATA] = FLASH_CMD_GOTOAPP;
  uval32 = CANMSG_SIGNATURE_RESET;
  memcpy(&tx_buff[CANMSG_OFFS_DATA + 1], &uval32, sizeof(uint32_t));
  if (CANBUS_Transfer(tx_buff, sizeof(uint8_t) * 2 + sizeof(uint32_t), NULL, 0) == false)
    return false;

  return true;
}

/** \brief Check 1 Flash page with CRC16
 *
 * \param [in] page Number of the page in Flash to check CRC
 * \param [in] crc Value of CRC (CRC16)
 * \return True if succeed
 *
 */
bool CANBUS_CheckCRC(uint8_t page, uint16_t crc)
{
  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_BOOTLOADER;
  tx_buff[CANMSG_OFFS_DATA] = FLASH_CMD_CHECKCRC;
  tx_buff[CANMSG_OFFS_DATA + 1] = (uint8_t)page;
  tx_buff[CANMSG_OFFS_DATA + 2] = 0;
  tx_buff[CANMSG_OFFS_DATA + 3] = (uint8_t)crc;
  tx_buff[CANMSG_OFFS_DATA + 4] = (uint8_t)(crc >> 8);
  if (CANBUS_Transfer(tx_buff, sizeof(uint8_t) * 2 + sizeof(uint16_t) * 2, CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return (CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1] == CANMSG_ANSWER_OK_SIGN);
}

/** \brief Write one page to Flash
 *
 * \param [in] page Page number
 * \return True if succeed
 *
 */
bool CANBUS_WritePage(uint8_t page)
{
  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_BOOTLOADER;
  tx_buff[CANMSG_OFFS_DATA] = FLASH_CMD_WRITEPAGE;
  tx_buff[CANMSG_OFFS_DATA + 1] = (uint8_t)page;
  tx_buff[CANMSG_OFFS_DATA + 2] = 0;
  if (CANBUS_Transfer(tx_buff, sizeof(uint8_t) * 2 + sizeof(uint16_t), CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return (CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1] == CANMSG_ANSWER_OK_SIGN);
}

/** \brief Write data to memory buffer of the bootloader
 *
 * \param [in] pos Position in buffer
 * \param [in] data Pointer to data to write (uint32_t)
 * \return True if succeed
 *
 */
bool CANBUS_WriteToBuff(uint8_t pos, uint32_t *data)
{
  tx_buff[CANMSG_OFFS_CMD] = CANMSG_CMD_BOOTLOADER;
  tx_buff[CANMSG_OFFS_DATA] = FLASH_CMD_SENDDATA;
  tx_buff[CANMSG_OFFS_DATA + 1] = pos;
  memcpy(&tx_buff[CANMSG_OFFS_DATA + 1], data, sizeof(uint32_t));
  if (CANBUS_Transfer(tx_buff, sizeof(uint8_t) * 3 + sizeof(uint32_t), CANBUS_RxBuffer, CANBUS_TIMEOUT) == false)
    return false;

  return (CANBUS_RxBuffer[CANMSG_OFFS_DATA + 1] == CANMSG_ANSWER_OK_SIGN);
}
