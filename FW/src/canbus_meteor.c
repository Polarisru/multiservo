#include "canbus.h"
#include "canbus_meteor.h"

static uint8_t tx_buff[CANBUS_MAX_LEN];
static uint8_t rx_buff[CANBUS_MAX_LEN];

/** \brief Send CAN packet and wait for reply if needed
 *
 * \param [in] tx_data Data buffer with packet to send
 * \param [in] tx_len Length of data packet to send
 * \param [out] rx_data Buffer with reply data
 * \param [in] timeout Timeout value in ms
 * \return True if reply was received
 *
 */
bool CANBUS_TransferMeteor(uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint8_t timeout)
{
  uint8_t errors = 0;
  uint32_t ticks;
  uint32_t id;
  uint8_t data[CANBUS_MAX_LEN];
  uint8_t cmd;
  TCanMsg rx_msg;

  while (errors < CANBUS_MAX_RETRIES)
  {
    /**< First empty FIFO to avoid duplicated messages from last communication */
    if (CANBUS_SendMessage(id, data, tx_len, true, false, false) == false)
      return false;

    /**< Wait till reply is received or timeout occurred */
    ticks = xTaskGetTickCount() + timeout;
    while (xTaskGetTickCount() < ticks)
    {
      if (CANBUS_ReceiveMessage(&rx_msg) == true)
      {

      }
      vTaskDelay(1);
    }
    errors++;
  }

  return false;
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
  if (addr >= 0x100)
  {
    return false;
  }

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
  if (addr >= 0x100)
  {
    return true;
  }

  return true;
}
