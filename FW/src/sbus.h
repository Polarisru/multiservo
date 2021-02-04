#ifndef SBUS_H
#define SBUS_H

#include "defines.h"

#define SBUS_CMD_START          0x0F
#define SBUS_CMD_STOP           0x00
#define SBUS_CMD_PROG           0xF9

#define SBUS_MAX_CHANNEL        16

#define SBUS_DATA_BITS          11
#define SBUS_VALUE_MASK         ((1 << SBUS_DATA_BITS) - 1)

#define SBUS_OFFS_CMD           0
#define SBUS_OFFS_DATA          1
#define SBUS_OFFS_FLAGS         23

#define SBUS_DATA_LEN           25
#define SBUS_PROG_LEN           17
#define SBUS_MAX_DATALEN        SBUS_DATA_LEN

#define SBUS_MIN_POSITION       -45.0f
#define SBUS_MAX_POSITION       45.0f
#define SBUS_MAX_VALUE          (1 << SBUS_DATA_BITS)

//void SBUS_Send(uint8_t *data, uint8_t len);
void SBUS_SetChannel(uint8_t channel, uint16_t value);
bool SBUS_SetPosition(float position);
void SBUS_SendCmd(void);
void SBUS_Enable(void);
void SBUS_Disable(void);
void SBUS_Task(void *pParameters);
void SBUS_Configuration(void);

#endif
