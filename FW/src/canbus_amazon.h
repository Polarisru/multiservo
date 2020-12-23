#ifndef CANBUS_AMAZON_H
#define CANBUS_AMAZON_H

#include "defines.h"

#define AMAZON_DFLT_NOMINAL_BAUDRATE    1000
#define AMAZON_DFLT_DATA_BAUDRATE       2000

#define AMAZON_CMD_SETPOS               22108
#define AMAZON_CMD_GETPOS               21000
#define AMAZON_CMD_GETPOS_REPLY         22101
#define AMAZON_CMD_STAT_REPLY           22102

#define AMAZON_DFLT_PRIO                0x14

#define AMAZON_BROADCAST_ID             0x7F

#define AMAZON_OFFS_ID                  0
#define AMAZON_OFFS_CMD                 8
#define AMAZON_OFFS_PRIO                24

#define AMAZON_TAIL_MASK                0x1F
#define AMAZON_TAIL_SIGN                0xC0

#define AMAZON_BUILD_ID(x)              ((AMAZON_DFLT_PRIO << AMAZON_OFFS_PRIO) | (x << AMAZON_OFFS_CMD) | (AMAZON_BROADCAST_ID << AMAZON_OFFS_ID))

#define AMAZON_GETPOS_CMD_SIZE          1
#define AMAZON_GETPOS_DATA_OFFS         2
#define AMAZON_SETPOS_CMD_SIZE          5

#define AMAZON_GETPOS_SYNC_ID           0x1D

bool CANBUS_SetPositionAmazon(float pos);
bool CANBUS_GetPositionAmazon(float *pos);
bool CANBUS_GetCurrentAmazon(uint8_t *val);
bool CANBUS_GetVoltageAmazon(uint8_t *val);
bool CANBUS_GetTemperatureAmazon(uint8_t *val);

#endif
