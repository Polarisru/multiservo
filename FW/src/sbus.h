#ifndef SBUS_H
#define SBUS_H

#include "defines.h"

#define SBUS_CMD_START          0xF0
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

void SBUS_Send(uint8_t *data, uint8_t len);
void SBUS_SendCmd(void);
void SBUS_Enable(void);
void SBUS_Disable(void);
void SBUS_Configuration(void);

#endif
