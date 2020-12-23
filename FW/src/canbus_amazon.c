#include "canbus.h"
#include "canbus_amazon.h"

static uint8_t tx_buff[CANBUS_MAX_LEN];
static uint8_t rx_buff[CANBUS_MAX_LEN];
static uint8_t canAmazonTransferID;
static uint8_t canAmazonPositionCounter;

static const uint8_t BitReverseTable256[] =
{
  0x00, 0x80, 0x40, 0xC0, 0x20, 0xA0, 0x60, 0xE0, 0x10, 0x90, 0x50, 0xD0, 0x30, 0xB0, 0x70, 0xF0,
  0x08, 0x88, 0x48, 0xC8, 0x28, 0xA8, 0x68, 0xE8, 0x18, 0x98, 0x58, 0xD8, 0x38, 0xB8, 0x78, 0xF8,
  0x04, 0x84, 0x44, 0xC4, 0x24, 0xA4, 0x64, 0xE4, 0x14, 0x94, 0x54, 0xD4, 0x34, 0xB4, 0x74, 0xF4,
  0x0C, 0x8C, 0x4C, 0xCC, 0x2C, 0xAC, 0x6C, 0xEC, 0x1C, 0x9C, 0x5C, 0xDC, 0x3C, 0xBC, 0x7C, 0xFC,
  0x02, 0x82, 0x42, 0xC2, 0x22, 0xA2, 0x62, 0xE2, 0x12, 0x92, 0x52, 0xD2, 0x32, 0xB2, 0x72, 0xF2,
  0x0A, 0x8A, 0x4A, 0xCA, 0x2A, 0xAA, 0x6A, 0xEA, 0x1A, 0x9A, 0x5A, 0xDA, 0x3A, 0xBA, 0x7A, 0xFA,
  0x06, 0x86, 0x46, 0xC6, 0x26, 0xA6, 0x66, 0xE6, 0x16, 0x96, 0x56, 0xD6, 0x36, 0xB6, 0x76, 0xF6,
  0x0E, 0x8E, 0x4E, 0xCE, 0x2E, 0xAE, 0x6E, 0xEE, 0x1E, 0x9E, 0x5E, 0xDE, 0x3E, 0xBE, 0x7E, 0xFE,
  0x01, 0x81, 0x41, 0xC1, 0x21, 0xA1, 0x61, 0xE1, 0x11, 0x91, 0x51, 0xD1, 0x31, 0xB1, 0x71, 0xF1,
  0x09, 0x89, 0x49, 0xC9, 0x29, 0xA9, 0x69, 0xE9, 0x19, 0x99, 0x59, 0xD9, 0x39, 0xB9, 0x79, 0xF9,
  0x05, 0x85, 0x45, 0xC5, 0x25, 0xA5, 0x65, 0xE5, 0x15, 0x95, 0x55, 0xD5, 0x35, 0xB5, 0x75, 0xF5,
  0x0D, 0x8D, 0x4D, 0xCD, 0x2D, 0xAD, 0x6D, 0xED, 0x1D, 0x9D, 0x5D, 0xDD, 0x3D, 0xBD, 0x7D, 0xFD,
  0x03, 0x83, 0x43, 0xC3, 0x23, 0xA3, 0x63, 0xE3, 0x13, 0x93, 0x53, 0xD3, 0x33, 0xB3, 0x73, 0xF3,
  0x0B, 0x8B, 0x4B, 0xCB, 0x2B, 0xAB, 0x6B, 0xEB, 0x1B, 0x9B, 0x5B, 0xDB, 0x3B, 0xBB, 0x7B, 0xFB,
  0x07, 0x87, 0x47, 0xC7, 0x27, 0xA7, 0x67, 0xE7, 0x17, 0x97, 0x57, 0xD7, 0x37, 0xB7, 0x77, 0xF7,
  0x0F, 0x8F, 0x4F, 0xCF, 0x2F, 0xAF, 0x6F, 0xEF, 0x1F, 0x9F, 0x5F, 0xDF, 0x3F, 0xBF, 0x7F, 0xFF
};

/** \brief Send RS485 packet and wait for reply if needed
 *
 * \param [in] id CAN ID
 * \param [in] tx_data Data buffer with packet to send
 * \param [in] tx_len Length of data packet to send
 * \param [out] rx_data Buffer with reply data
 * \param [in] timeout Timeout value in ms
 * \return True if reply was received
 *
 */
 bool CANBUS_TransferAmazon(uint32_t id, uint8_t *tx_data, uint8_t tx_len, uint8_t *rx_data, uint16_t rx_cmd, uint8_t timeout)
{
  uint8_t errors = 0;
  CANFDMessage tx_msg;
  CANFDMessage rx_msg;
  uint32_t ticks;
  uint16_t cmd;

  tx_msg.id = id;
  memcpy(tx_msg.data, tx_data, tx_len);
  /**< Add tail to every Amazon message */
  tx_msg.data[tx_len] = (canAmazonTransferID++ & AMAZON_TAIL_MASK) | AMAZON_TAIL_SIGN;
  tx_msg.len = tx_len + 1;
  //tx_msg.ext = true;
  //tx_msg.type = MCP2518FD_CANFD_WITH_BIT_RATE_SWITCH;

  while (errors < CANBUS_MAX_RETRIES)
  {
    /**< First empty FIFO to avoid duplicated messages from last communication */
    //MCP2518FD_ResetRxFifo();
    if (CANBUS_SendMessage(&tx_msg, true, true, true) == false)
    //if (MCP2518FD_SendMsg(&tx_msg) == false)
      return false;
    /**< Don't wait for reply, just go out */
    if (timeout == 0)
      return true;
    /**< Wait till reply is received or timeout occurred */
    ticks = xTaskGetTickCount() + timeout;
    while (xTaskGetTickCount() < ticks)
    {
      //if (MCP2518FD_ReceiveMsg(&rx_msg) == true)
      if (CANBUS_ReceiveMessage(&rx_msg) == true)
      {
        cmd = (uint16_t)(rx_msg.id >> AMAZON_OFFS_CMD);
        if (cmd == rx_cmd)
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

/** \brief Set servo position for Amazon protocol
 *
 * \param [in] pos New position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_SetPositionAmazon(float pos)
{
  int16_t val;
  uint32_t id;
  uint16_t u16_val;
  uint8_t offs;
  const uint8_t cmd_mask[AMAZON_SETPOS_CMD_SIZE] = {0x10, 0x00, 0xC0, 0xFF, 0x1F};

  if (pos < -SERVO_MAX_ANGLE)
    pos = -SERVO_MAX_ANGLE;
  if (pos > SERVO_MAX_ANGLE)
    pos = SERVO_MAX_ANGLE;

  /**< Angle in mrads */
  val = (int16_t)(pos * M_PI / 180 * 1000);
  id = AMAZON_BUILD_ID(AMAZON_CMD_SETPOS);
  /**< Convert position to command format */
  memset(tx_buff, 0, AMAZON_SETPOS_CMD_SIZE);
  u16_val = ((uint16_t)BitReverseTable256[(uint8_t)((uint16_t)val >> 8)] << 8) + BitReverseTable256[(uint8_t)((uint16_t)val & 0xff)];
  offs = 5;
  for (uint8_t i = 0; i < 16; i++)
  {
    if (u16_val & 0x0001)
      tx_buff[offs / 8] |= (1 << (offs % 8));
    offs++;
    u16_val >>= 1;
  }
  for (uint8_t i = 0; i < AMAZON_SETPOS_CMD_SIZE; i++)
    tx_buff[i] |= cmd_mask[i];
  for (uint8_t i = 0; i < AMAZON_SETPOS_CMD_SIZE; i++)
    tx_buff[i] = BitReverseTable256[tx_buff[i]];

  if (CANBUS_TransferAmazon(id, tx_buff, AMAZON_SETPOS_CMD_SIZE, rx_buff, 0, 0) == false)
    return false;

  return true;
}

/** \brief Read current servo position for Amazon protocol
 *
 * \param [out] pos Pointer to position in degrees
 * \return True if succeed
 *
 */
bool CANBUS_GetPositionAmazon(float *pos)
{
  int16_t val;
  uint32_t id;

  id = AMAZON_BUILD_ID(AMAZON_CMD_GETPOS);
  tx_buff[0] = canAmazonPositionCounter & 0x3F;
  /**< Hack to avoid receiving servo parameters as reply */
  if (tx_buff[0] == AMAZON_GETPOS_SYNC_ID)
    tx_buff[0]++;
  if (CANBUS_TransferAmazon(id, tx_buff, AMAZON_GETPOS_CMD_SIZE, rx_buff, AMAZON_CMD_GETPOS_REPLY, CANBUS_TIMEOUT) == false)
    return false;

  val = (int16_t)(((uint16_t)rx_buff[AMAZON_GETPOS_DATA_OFFS + 1] << 8) + rx_buff[AMAZON_GETPOS_DATA_OFFS]);
  *pos = (float)val * 180 / M_PI / 1000;

  return true;
}

/** \brief Read status parameters from servo
 *
 * \param [in] offset Offset of parameter in data packet
 * \param [in] len Length of parameter
 * \param [out] val Pointer to valur for readout
 * \return True if succeed
 *
 */
bool CANBUS_GetParameter(uint8_t offset, uint8_t len, void *val)
{
  uint32_t id;
  CANFDMessage rx_msg;
  float flVal;

  id = AMAZON_BUILD_ID(AMAZON_CMD_GETPOS);
  tx_buff[0] = AMAZON_GETPOS_SYNC_ID & 0x3F;
  if (CANBUS_TransferAmazon(id, tx_buff, AMAZON_GETPOS_CMD_SIZE, rx_buff, AMAZON_CMD_STAT_REPLY, CANBUS_TIMEOUT) == false)
    return false;

  memcpy(val, &rx_buff[offset], len);

  /**< Next message is coming immediately */
  CANBUS_ReceiveMessage(&rx_msg);

  /**< Dummy read to increment counter */
  if (CANBUS_GetPositionAmazon(&flVal) == false)
    return false;

  return true;
}

/** \brief Get current value from servo
 *
 * \param [out] value Pointer to value for voltage readout
 * \return True if succeed
 *
 */
bool CANBUS_GetCurrentAmazon(uint8_t *val)
{
  uint16_t current;

  if (CANBUS_GetParameter(0, sizeof(uint16_t), (void*)&current) == false)
    return false;

  *val = (uint8_t)(current / 20);

  return true;
}

/** \brief Get voltage value from servo
 *
 * \param [out] value Pointer to value for voltage readout
 * \return True if succeed
 *
 */
bool CANBUS_GetVoltageAmazon(uint8_t *val)
{
  uint8_t voltage;

  if (CANBUS_GetParameter(2, sizeof(uint8_t), (void*)&voltage) == false)
    return false;

  *val = voltage / 2;

  return true;
}

/** \brief Get temperature value from servo
 *
 * \param [out] value Pointer to value for voltage readout
 * \return True if succeed
 *
 */
bool CANBUS_GetTemperatureAmazon(uint8_t *val)
{
  int8_t temperature;

  if (CANBUS_GetParameter(3, sizeof(uint8_t), (void*)&temperature) == false)
    return false;

  *val = (uint8_t)(temperature + 50);

  return true;
}
