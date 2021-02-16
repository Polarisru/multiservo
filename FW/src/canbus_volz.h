#ifndef CANBUS_VOLZ_H
#define CANBUS_VOLZ_H

#include "defines.h"

#define CANBUS_CAN_ID             0x3F0

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
#define CANMSG_CMD_GET_HWFW       0x07
#define CANMSG_CMD_SET_GROUP_POS  0x08
#define CANMSG_CMD_GET_STRING     0x09
#define CANMSG_CMD_SET_STRING     0x0A
#define CANMSG_CMD_EXECUTE        0x0B
#define CANMSG_CMD_READ_EE        0x3D
#define CANMSG_CMD_WRITE_EE       0x3E
#define CANMSG_CMD_BOOTLOADER     0x3F

#define CANMSG_ID_WIDTH           4
#define CANMSG_ID_MASK            ((1 << CANMSG_ID_WIDTH) - 1)
#define CANMSG_ID_BROADCAST       0x00

#define CANMSG_CMD_REPLY_BIT      (1 << 7)
#define CANMSG_CMD_WIDTH          6
#define CANMSG_CMD_MASK           ((1 << CANMSG_CMD_WIDTH) - 1)

#define CANMSG_PARAM_ID_OFFS      0
#define CANMSG_PARAM_CMD_OFFS     1
#define CANMSG_PARAM_DATA_OFFS    2

#define CANMSG_PARAM_ID_WIDTH     7
#define CANMSG_PARAM_ID_MASK      ((1 << CANMSG_PARAM_ID_WIDTH) - 1)

#define CANMSG_PARAM_TYPE_OFFS    0
#define CANMSG_PARAM_TYPE_WIDTH   4
#define CANMSG_PARAM_TYPE_MASK    ((1 << CANMSG_PARAM_TYPE_WIDTH) - 1)
#define CANMSG_PARAM_OPCODE_OFFS  4
#define CANMSG_PARAM_OPCODE_WIDTH 2
#define CANMSG_PARAM_OPCODE_MASK  ((1 << CANMSG_PARAM_OPCODE_WIDTH) - 1)

#define CANMSG_PARAM_VAL          0
#define CANMSG_PARAM_MINVAL       1
#define CANMSG_PARAM_MAXVAL       2
#define CANMSG_PARAM_DFLTVAL      3

#define CANMSG_PARAM_NAME_BIT     (1 << 7)

#define CANMSG_EXECUTE_NORMMODE   0
#define CANMSG_EXECUTE_FREE       1
#define CANMSG_EXECUTE_ROTCW      2
#define CANMSG_EXECUTE_ROTCCW     3
#define CANMSG_EXECUTE_NEUTRAL    4
#define CANMSG_EXECUTE_SAVESTR    5
#define CANMSG_EXECUTE_RESETSTS   6
#define CANMSG_EXECUTE_MOTORTEST  7

#define CANMSG_EXECUTE_REPLY_OK   0xAA
#define CANMSG_EXECUTE_REPLY_ERR  0xEE

#define CANMSG_SIGNATURE_START    0xAA55A55A
#define CANMSG_SIGNATURE_RESET    0x12345678

/**< List of bootloader commands */
#define FLASH_CMD_STARTBL       0x00
#define FLASH_CMD_GOTOAPP       0x01
#define FLASH_CMD_SENDDATA      0x02
#define FLASH_CMD_WRITEPAGE     0x03
#define FLASH_CMD_EEWRITE       0x04
#define FLASH_CMD_CHECKCRC      0x05
#define FLASH_CMD_GETINFO       0x06

/**< Bootloader reply */
#define CANMSG_ANSWER_OK_SIGN     0xAA

bool CANBUS_SetPosition(float pos);
bool CANBUS_GetPosition(float *pos);
bool CANBUS_ReadByte(uint16_t addr, uint8_t *value);
bool CANBUS_WriteByte(uint16_t addr, uint8_t value);
bool CANBUS_GetCurrent(uint8_t *value);
bool CANBUS_GetVoltage(uint8_t *value);
bool CANBUS_GetTemperature(uint8_t *value);
bool CANBUS_StartBL(void);
bool CANBUS_GoToApp(void);
bool CANBUS_CheckCRC(uint8_t page, uint16_t crc);
bool CANBUS_WritePage(uint8_t page);
bool CANBUS_WriteToBuff(uint8_t pos, uint32_t *data);

#endif
