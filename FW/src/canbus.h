#ifndef CANBUS_H
#define CANBUS_H

#include "defines.h"
#include "driver_can.h"

#define CANBUS_BUF_LEN            8
#define CANBUS_ID_MASK            0x7FF
#define CANBUS_BACKDOOR_ID        0x7F0
#define CANBUS_ID_OWN_MASK        0x7F0

#define CANMSG_OFFSET_CMD         0
#define CANMSG_OFFSET_DATA        1

#define CANMSG_CMD_ECHO           0x00
#define CANMSG_CMD_SET_POS        0x01
#define CANMSG_CMD_GET_POS        0x02
#define CANMSG_CMD_GET_STAT       0x03
#define CANMSG_CMD_GET_TIME       0x04
#define CANMSG_CMD_GET_PARAM      0x05
#define CANMSG_CMD_SET_PARAM      0x06
#define CANMSG_CMD_GET_NAME       0x07
#define CANMSG_CMD_SET_NAME       0x08
#define CANMSG_CMD_GET_SN         0x09
#define CANMSG_CMD_SET_SN         0x0A
#define CANMSG_CMD_READ_EE        0x0B
#define CANMSG_CMD_WRITE_EE       0x0C
#define CANMSG_CMD_BOOTLOADER     0x3F

#define CANMSG_ID_WIDTH           4
#define CANMSG_ID_MASK            ((1 << CANMSG_ID_WIDTH) - 1)
#define CANMSG_ID_BROADCAST       0x00

#define CANMSG_CMD_REPLY_BIT      (1 << 7)
#define CANMSG_CMD_GROUP_BIT      (1 << 6)
#define CANMSG_CMD_WIDTH          6
#define CANMSG_CMD_MASK           ((1 << CANMSG_CMD_WIDTH) - 1)

#define FLASH_CMD_STARTBL         0x00

#define CANMSG_PARAM_WIDTH        6
#define CANMSG_PARAM_MASK         ((1 << CANMSG_PARAM_WIDTH) - 1)
#define CANMSG_PARAM_VAL          0
#define CANMSG_PARAM_MINVAL       1
#define CANMSG_PARAM_MAXVAL       2
#define CANMSG_PARAM_NAME         3


typedef struct
{
  uint32_t  id;
  uint8_t   data[CANBUS_BUF_LEN];
  uint8_t   len;
  enum can_format fmt;
} TCanMsg;

bool CANBUS_Send(uint32_t Id, uint8_t *data, uint8_t datalen);
bool CANBUS_SetBaudrate(uint32_t nominal_baudrate, uint32_t data_baudrate);

#endif
