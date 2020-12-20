#ifndef ACTIONS_H
#define ACTIONS_H

#include "defines.h"

bool ACTION_MoveToPosition(float pos);
bool ACTION_GetPosition(float *pos);
bool ACTION_ReadByte(uint16_t addr, uint8_t *value);
bool ACTION_WriteByte(uint16_t addr, uint8_t value);
bool ACTION_SetMode(uint8_t mode);

#endif
