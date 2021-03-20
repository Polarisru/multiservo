#ifndef ACTIONS_H
#define ACTIONS_H

#include "defines.h"

enum {
  ACTION_PARAM_CURRENT,
  ACTION_PARAM_VOLTAGE,
  ACTION_PARAM_TEMPERATURE,
  ACTION_PARAM_LAST
};

bool ACTIONS_MoveToPosition(float pos);
bool ACTIONS_GetPosition(float *pos);
bool ACTIONS_ReadByte(uint16_t addr, uint8_t *value);
bool ACTIONS_WriteByte(uint16_t addr, uint8_t value);
bool ACTION_ReadParameter(uint8_t param, uint8_t *val);
bool ACTIONS_SetId(uint8_t id);
bool ACTIONS_SetMode(uint8_t mode);

#endif
