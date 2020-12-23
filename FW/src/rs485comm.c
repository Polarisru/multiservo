#include "rs485cmd.h"
#include "rs485comm.h"
#include "rs485.h"

uint8_t RS485COMM_RxBuffer[RS485_PAKET_SIZE];
uint8_t RS485COMM_TxBuffer[RS485_PAKET_SIZE];

/** \brief Set servo position
 *
 * \param [in] pos New position in degrees
 * \return true if succeed
 *
 */
bool RS485COMM_SetPosition(float pos)
{
  int16_t val;

  if (pos < -SERVO_MAX_ANGLE)
    pos = -SERVO_MAX_ANGLE;
  if (pos > SERVO_MAX_ANGLE)
    pos = SERVO_MAX_ANGLE;

  val = (int16_t)(pos * SERVO_RAW_ANGLE_SCALE / SERVO_ANGLE_SCALE);

  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_SETPOS170;
  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = (uint8_t)(val >> 8);
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = (uint8_t)val;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_SETPOS170, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  //if (RS485COMM_RxBuffer[RS485_OFFS_CMD] != RS485_ANS_SETPOS170)
  //  return false;

  return true;
}

/** \brief Read current servo position
 *
 * \param [out] pos Pointer to position in degrees
 * \return true if succeed
 *
 */
bool RS485COMM_GetPosition(float *pos)
{
  int16_t val;

  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_GETPOS170;
  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = 0;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = 0;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_GETPOS170, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  //if (RS485COMM_RxBuffer[RS485_OFFS_CMD] != RS485_ANS_GETPOS170)
  //  return false;

  val = (int16_t)(((uint16_t)RS485COMM_RxBuffer[RS485_OFFS_DATA] << 8) + RS485COMM_RxBuffer[RS485_OFFS_DATA + 1]);
  *pos = (float)val * SERVO_ANGLE_SCALE / SERVO_RAW_ANGLE_SCALE;

  return true;
}

/** \brief Read byte from servo EEPROM
 *
 * \param [in] addr Address to read from
 * \param [out] value Pointer to store read value
 * \return true if succeed
 *
 */
bool RS485COMM_ReadByte(uint16_t addr, uint8_t *value)
{
  uint8_t resp;

  if (addr >= 0x100)
  {
    /**< High area needs another command */
    RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_READ_EEHIGH;
    resp = RS485_ANS_READ_EEHIGH;
  } else
  {
    RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_READ_EELOW;
    resp = RS485_ANS_READ_EELOW;
  }
  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = (uint8_t)addr;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = (uint8_t)addr;

  if (RS485_Transfer(RS485COMM_TxBuffer, resp, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  if (/*(RS485COMM_RxBuffer[RS485_OFFS_CMD] != resp) ||*/ (RS485COMM_TxBuffer[RS485_OFFS_DATA] != RS485COMM_TxBuffer[RS485_OFFS_DATA + 1]))
    return false;

  *value = RS485COMM_RxBuffer[RS485_OFFS_DATA + 1];

  return true;
}

/** \brief Write byte to servo EEPROM
 *
 * \param [in] addr Address to write to
 * \param [in] value Value to write
 * \return true if succeed
 *
 */
bool RS485COMM_WriteByte(uint16_t addr, uint8_t value)
{
  uint8_t resp;

  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_ACCESSBIT;
  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = RS485_ACCESS_SIGN1;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = RS485_ACCESS_SIGN2;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_ACCESSBIT, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  if (/*(RS485COMM_RxBuffer[RS485_OFFS_CMD] != RS485_ANS_ACCESSBIT) ||*/ (RS485COMM_RxBuffer[RS485_OFFS_DATA] != RS485_ACCESS_ANS1) ||
      (RS485COMM_RxBuffer[RS485_OFFS_DATA + 1] != RS485_ACCESS_ANS2))
    return false;

  if (addr >= 0x100)
  {
    /**< High area needs another command */
    RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_WRITE_EEHIGH;
    resp = RS485_ANS_WRITE_EEHIGH;
  } else
  {
    RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_WRITE_EELOW;
    resp = RS485_ANS_WRITE_EELOW;
  }
  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = (uint8_t)addr;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = (uint8_t)value;
  if (RS485_Transfer(RS485COMM_TxBuffer, resp, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  if (/*(RS485COMM_RxBuffer[RS485_OFFS_CMD] != resp) ||*/ (RS485COMM_TxBuffer[RS485_OFFS_DATA] != (uint8_t)addr))
    return false;

  return true;
}

/** \brief Read raw magnet sensor value from servo
 *
 * \param [out] pos Pointer to sensor value readout
 * \return true if succeed
 *
 */
bool RS485COMM_GetSensor(uint16_t *value)
{
  int16_t val;

  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_GETMAGNET;

  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = 0;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = 0;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_GETMAGNET, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  val = ((uint16_t)RS485COMM_RxBuffer[RS485_OFFS_DATA] << 8) + RS485COMM_RxBuffer[RS485_OFFS_DATA + 1];
  *value = val;

  return true;
}

/** \brief Read current value from servo
 *
 * \param value Pointer to current value readout
 * \return True if succeed
 *
 */
bool RS485COMM_GetCurrent(uint8_t *value)
{
  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_GETCURRENT;

  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = 0;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = 0;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_GETCURRENT, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  /**< 1A = 50 units */
  *value = RS485COMM_RxBuffer[RS485_OFFS_DATA];

  return true;
}

/** \brief Read voltage value from servo
 *
 * \param value Pointer to voltage value readout
 * \return True if succeed
 *
 */
bool RS485COMM_GetVoltage(uint8_t *value)
{
  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_GETVOLTAGE;

  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = 0;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = 0;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_GETVOLTAGE, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  /**< 1V = ? */
  *value = RS485COMM_RxBuffer[RS485_OFFS_DATA];

  return true;
}

/** \brief Read temperature value from servo
 *
 * \param value Pointer to temperature value readout
 * \return True if succeed
 *
 */
bool RS485COMM_GetTemperature(uint8_t *value)
{
  RS485COMM_TxBuffer[RS485_OFFS_CMD] = RS485_CMD_GETTEMP;

  RS485COMM_TxBuffer[RS485_OFFS_ID] = RS485COMM_BROADCAST_ID;
  RS485COMM_TxBuffer[RS485_OFFS_DATA] = 0;
  RS485COMM_TxBuffer[RS485_OFFS_DATA + 1] = 0;
  if (RS485_Transfer(RS485COMM_TxBuffer, RS485_ANS_GETTEMP, RS485COMM_RxBuffer, RS485COMM_TIMEOUT) == false)
    return false;

  /**< (3900 - temperature_ADC) / 20, must be (-11.69*t + 1.8663) *2 /5 *4096 */
  *value = RS485COMM_RxBuffer[RS485_OFFS_DATA];

  return true;
}
