#ifndef _RS485COMM_H
#define _RS485COMM_H

#include "defines.h"

#define RS485COMM_TIMEOUT           10 // 10 ms for writing timeout

#define RS485COMM_BROADCAST_ID      0x1F

bool RS485COMM_SetPosition(float pos);
bool RS485COMM_GetPosition(float *pos);
bool RS485COMM_ReadByte(uint16_t addr, uint8_t *value);
bool RS485COMM_WriteByte(uint16_t addr, uint8_t value);
bool RS485COMM_GetSensor(uint16_t *value);
bool RS485COMM_GetCurrent(uint8_t *value);
bool RS485COMM_GetVoltage(uint8_t *value);
bool RS485COMM_GetTemperature(uint8_t *value);

#endif
