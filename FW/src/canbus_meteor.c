#include "canbus.h"
#include "canbus_meteor.h"

static uint8_t tx_buff[CANBUS_MAX_LEN];
static uint8_t rx_buff[CANBUS_MAX_LEN];

uint32_t CANBUS_BuildRequestID(uint8_t cmd)
{
  CANBUS_IdType newID;

  newID.value = 0;
  newID.bit.SRC = 0;
  newID.bit.LCL = 1;
  newID.bit.SP2 = 1;
  newID.bit.SID = CANBUS_BROADCAST;
  newID.bit.OC = cmd & ((1 << CANBUS_LEN_CMD) - 1);

  return newID.value;
}

/** \brief Build ID for servo reply
 *
 * \param [in] id ID of original command
 * \param [in] cmd Command for reply
 * \return Reply ID as uint32_t
 *
 */
uint32_t CANBUS_BuildReplyID(uint32_t id, uint8_t cmd)
{
  CANBUS_IdType newID;

  newID.value = id;
  newID.bit.SRC = CANBUS_SRC_SERVO;
  newID.bit.SID = CANBUS_BROADCAST;
  newID.bit.OC = cmd & ((1 << CANBUS_LEN_CMD) - 1);

  return newID.value;
}

/** \brief Send CAN packet and wait for reply if needed
 *
 * \param [in] tx_data Data buffer with packet to send
 * \param [in] tx_len Length of data packet to send
 * \param [out] rx_data Buffer with reply data
 * \param [in] timeout Timeout value in ms
 * \return True if reply was received
 *
 */
bool CANBUS_TransferMeteor(uint32_t id, uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint8_t timeout)
{
  uint8_t errors = 0;
  uint32_t ticks;
  CANBUS_IdType reply_id;
  uint8_t cmd;
  TCanMsg rx_msg;

  reply_id.value = id;
  cmd = reply_id.bit.OC + 1;
  while (errors < CANBUS_MAX_RETRIES)
  {
    /**< First empty FIFO to avoid duplicated messages from last communication */
    if (CANBUS_SendMessage(id, tx_data, tx_len, true, false, false) == false)
      return false;

    /**< Wait till reply is received or timeout occurred */
    ticks = xTaskGetTickCount() + timeout;
    while (xTaskGetTickCount() < ticks)
    {
      if (CANBUS_ReceiveMessage(&rx_msg) == true)
      {
        reply_id.value = rx_msg.id;
        if (reply_id.bit.OC == cmd)
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

/** \brief Build ID for servo reply for SETID command
 *
 * \param [in] id ID of original command
 * \param [in] cmd Command for reply
 * \return Reply ID as uint32_t
 *
 */
//uint32_t CANBUS_BuildReplySetID(uint32_t id, uint8_t cmd)
//{
//  CANBUS_IdType newID;
//
//  newID.value = id;
//  newID.bit.SRC = CANBUS_SRC_SERVO;
//  newID.bit.SID = CANBUS_BROADCAST;
//  newID.bit.OC = cmd & ((1 << CANBUS_LEN_CMD) - 1);
//
//  return newID.value;
//}

/** \brief Set servo position for Meteor protocol
 *
 * \param [in] pos New position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_SetPositionMeteor(float pos)
{
  int16_t i;
  uint32_t id;

  id = CANBUS_BuildRequestID(CAN_CMD_SETPOS);
  i = (int16_t)(pos * 11.3778f);
  tx_buff[0] = (uint8_t)(i >> 8);
  tx_buff[1] = (uint8_t)i;

  if (CANBUS_TransferMeteor(id, tx_buff, 2, rx_buff, CANBUS_TIMEOUT) == false)
    return false;

  return true;
}

/** \brief Read current servo position for Meteor protocol
 *
 * \param [out] pos Pointer to position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_GetPositionMeteor(float *pos)
{
  *pos = 0.0f;
  return true;
}

/** \brief Read byte from servo EEPROM
 *
 * \param [in] addr Address to read from
 * \param [out] value Pointer to store read value
 * \return True if succeed
 *
 */
bool CANBUS_ReadByteMeteor(uint16_t addr, uint8_t *value)
{
  uint32_t id;

  if (addr >= 0x100)
  {
    return false;
  }

  id = CANBUS_BuildRequestID(CAN_CMD_READEEPROM);
  tx_buff[0] = (uint8_t)addr;

  if (CANBUS_TransferMeteor(id, tx_buff, 1, rx_buff, CANBUS_TIMEOUT) == false)
    return false;

  *value = rx_buff[1];

  return true;
}

/** \brief Write byte to servo EEPROM
 *
 * \param [in] addr Address to write to
 * \param [in] value Value to write
 * \return True if succeed
 *
 */
bool CANBUS_WriteByteMeteor(uint16_t addr, uint8_t value)
{
  uint32_t id;

  if (addr >= 0x100)
  {
    return true;
  }

  id = CANBUS_BuildRequestID(CAN_CMD_WRITEEEPROM);
  tx_buff[0] = (uint8_t)addr;
  tx_buff[1] = (uint8_t)value;

  if (CANBUS_TransferMeteor(id, tx_buff, 2, rx_buff, CANBUS_TIMEOUT) == false)
    return false;

  return (rx_buff[1] == value);
}

/** \brief Just send bootloader starting command
 *
 * \return True if succeed
 *
 */
bool CANBUS_StartBLMeteor(void)
{
  uint32_t uval32;

  id = CANBUS_BuildRequestID(CAN_CMD_STARTBL);

  uval32 = CANMSG_SIGNATURE_START;
  tx_buff[0] = 0;
  memcpy(&tx_buff[1], &uval32, sizeof(uint32_t));
  if (CANBUS_Transfer(id, tx_buff, sizeof(uint8_t) * 1 + sizeof(uint32_t), rx_buff, CANBUS_TIMEOUT) == false)
    return false;

  return true;
}

/** \brief Go to application
 *
 * \return True if succeed
 *
 */
bool CANBUS_GoToAppMeteor(void)
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
