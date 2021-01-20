#include "actions.h"
#include "canbus.h"
#include "canbus_volz.h"
#include "canbus_amazon.h"
#include "conversion.h"
#include "global.h"
#include "keeloq.h"
#include "pwmout.h"
#include "rs485comm.h"
#include "sbus.h"

bool ACTIONS_MoveToPosition(float pos)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      PWMOUT_SetValue(pos);
      return true;
    case CONN_MODE_RS485:
      return RS485COMM_SetPosition(pos);
    case CONN_MODE_CAN:
      return CANBUS_SetPosition(pos);
    case CONN_MODE_UAVCAN:
      return CANBUS_SetPositionAmazon(pos);
    case CONN_MODE_SBUS:
      break;
    default:
      return false;
  }

  return false;
}

bool ACTIONS_GetPosition(float *pos)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      /**< Get feedback voltage */
      *pos = CONVERSION_GetFeedbackPos();
      return true;
    case CONN_MODE_RS485:
      return RS485COMM_GetPosition(pos);
    case CONN_MODE_CAN:
      return CANBUS_GetPosition(pos);
    case CONN_MODE_UAVCAN:
      return CANBUS_GetPositionAmazon(pos);
    case CONN_MODE_SBUS:
      break;
  }

  return false;
}

bool ACTIONS_ReadByte(uint16_t addr, uint8_t *value)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      return KEELOQ_Read(addr, value);
    case CONN_MODE_RS485:
      return RS485COMM_ReadByte(addr, value);
    case CONN_MODE_CAN:
      return CANBUS_ReadByte(addr, value);
    case CONN_MODE_UAVCAN:
      break;
    case CONN_MODE_SBUS:
      break;
  }

  return false;
}

bool ACTIONS_WriteByte(uint16_t addr, uint8_t value)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      return KEELOQ_Write(addr, value);
    case CONN_MODE_RS485:
      return RS485COMM_WriteByte(addr, value);
    case CONN_MODE_CAN:
      return CANBUS_WriteByte(addr, value);
    case CONN_MODE_UAVCAN:
      break;
    case CONN_MODE_SBUS:
      break;
  }

  return false;
}

/** \brief Read device parameter
 *
 * \param param Id of the parameter
 * \param val Pointer to uint8_t to store result
 * \return True if success
 *
 */
bool ACTION_ReadParameter(uint8_t param, uint8_t *val)
{
  uint8_t temp;
  bool result;

  if (param >= ACTION_PARAM_LAST)
    return false;

  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      switch (param)
      {
        case ACTION_PARAM_CURRENT:
          temp = KEELOQ_ADDR_CURRENT;
          break;
        case ACTION_PARAM_VOLTAGE:
          temp = KEELOQ_ADDR_VOLTAGE;
          break;
        case ACTION_PARAM_TEMPERATURE:
          temp = KEELOQ_ADDR_TEMP;
          break;
        default:
          return false;
      }
      //PWM_DisableChannel(PWM_CH1);
      result = KEELOQ_Read(temp, val);
      //PWM_EnableChannel(PWM_CH1);
      if (*val == 0xff)
        return false;
      return result;

    case CONN_MODE_RS485:
      switch (param)
      {
        case ACTION_PARAM_CURRENT:
          return RS485COMM_GetCurrent(val);
        case ACTION_PARAM_VOLTAGE:
          return RS485COMM_GetVoltage(val);
        case ACTION_PARAM_TEMPERATURE:
          return RS485COMM_GetTemperature(val);
      }

    case CONN_MODE_CAN:
      switch (param)
      {
        case ACTION_PARAM_CURRENT:
          return CANBUS_GetCurrent(val);
        case ACTION_PARAM_VOLTAGE:
          return CANBUS_GetVoltage(val);
        case ACTION_PARAM_TEMPERATURE:
          return CANBUS_GetTemperature(val);
      }

    case CONN_MODE_UAVCAN:
      switch (param)
      {
        case ACTION_PARAM_CURRENT:
          return CANBUS_GetCurrentAmazon(val);
        case ACTION_PARAM_VOLTAGE:
          return CANBUS_GetVoltageAmazon(val);
        case ACTION_PARAM_TEMPERATURE:
          return CANBUS_GetTemperatureAmazon(val);
          break;
      }
      return true;
  }

  return false;
}

/** \brief Set working mode (PWM/RS485/CAN/SBUS)
 *
 * \param [in] mode Working mode
 * \return True if succeed
 *
 */
bool ACTIONS_SetMode(uint8_t mode)
{
  if (mode >= CONN_MODE_LAST)
    return false;

  PWMOUT_Disable();

  GLOBAL_ConnMode = mode;
  switch (mode)
  {
    case CONN_MODE_PWM:
      PWMOUT_Enable();
      break;
    case CONN_MODE_RS485:
      /**< Do nothing, is always working */
      break;
    case CONN_MODE_CAN:
      break;
    case CONN_MODE_UAVCAN:
      break;
    case CONN_MODE_SBUS:
      SBUS_Enable();
      break;
  }

  return true;
}

