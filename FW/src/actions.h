#ifndef ACTIONS_H
#define ACTIONS_H

#include "defines.h"

bool ACTIONS_MoveToPosition(float pos);
bool ACTIONS_GetPosition(float *pos);
bool ACTIONS_ReadByte(uint16_t addr, uint8_t *value);
bool ACTIONS_WriteByte(uint16_t addr, uint8_t value);
bool ACTIONS_SetMode(uint8_t mode);

#endif
