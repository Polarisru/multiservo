#include "actions.h"
#include "canbus.h"
#include "global.h"
#include "pwmout.h"

bool ACTIONS_MoveToPosition(float pos)
{
  switch (GLOBAL_ConnMode)
  {
    case CONN_MODE_PWM:
      PWMOUT_SetValue(pos);
      break;
    case CONN_MODE_RS485:
      break;
    case CONN_MODE_CAN:
      break;
    case CONN_MODE_UAVCAN:
      break;
    case CONN_MODE_SBUS:
      break;
    default:
      return false;
  }

  return true;
}

bool ACTIONS_GetPosition(float *pos)
{
  return true;
}

bool ACTIONS_ReadByte(uint16_t addr, uint8_t *value)
{
  return true;
}

bool ACTIONS_WriteByte(uint16_t addr, uint8_t value)
{
  return true;
}

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
      break;
    case CONN_MODE_CAN:
      break;
    case CONN_MODE_UAVCAN:
      break;
    case CONN_MODE_SBUS:
      break;
  }

  return true;
}

