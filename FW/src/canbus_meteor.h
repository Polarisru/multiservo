#ifndef CANBUS_METEOR_H
#define CANBUS_METEOR_H

#include "defines.h"

/**< Structure of Meteor ID */
#define CANBUS_OFFS_CMD       2
#define CANBUS_LEN_CMD        6
#define CANBUS_MASK_CMD       (((1 << CANBUS_LEN_CMD) - 1) << CANBUS_OFFS_CMD)

#define CANBUS_OFFS_ID        8
#define CANBUS_LEN_ID         4
#define CANBUS_MASK_ID        (((1 << CANBUS_LEN_ID) - 1) << CANBUS_OFFS_ID)

#define CANBUS_OFFS_SRC       13
#define CANBUS_LEN_SRC        2
#define CANBUS_MASK_SRC       (((1 << CANBUS_LEN_SRC) - 1) << CANBUS_OFFS_SRC)

#define CANBUS_BROADCAST      0x00

/**< Source id for servo */
#define CANBUS_SRC_SERVO      0x03

#define CANBUS_METEOR_MASK    0x1FFF9F00
#define CANBUS_NOC_FILTER     0x08529000
#define CANBUS_TMC_FILTER     0x18529000
#define CANBUS_SERVO_MAX_ID   0x00004FFF

/**< List of the CAN commands (OC), only requests listed, for replies add 1  to responsible command */
#define CAN_CMD_SETPOS        6
#define CAN_CMD_GETPOS        7
#define CAN_CMD_GETCOUNTERS   8
#define CAN_CMD_GETSTATUS     10
#define CAN_CMD_SETID         20
#define CAN_CMD_GETVER        24
#define CAN_CMD_RESETCOUNTERS 26
#define CAN_CMD_GETWORKTIME   28
#define CAN_CMD_GETIBIT       30
#define CAN_CMD_GETDIGINPUT   34
#define CAN_CMD_SETDIGOUT     36
#define CAN_CMD_WRITEACCESS   38
#define CAN_CMD_READEEPROM    40
#define CAN_CMD_WRITEEEPROM   42
#define CAN_CMD_SETPOSOFFSET  44
#define CAN_CMD_SETROLE       46
#define CAN_CMD_RESETBITS     48
#define CAN_CMD_TRANSIT       50
#define CAN_CMD_STARTBL       56
#define CAN_CMD_GOTOAPP       57

#define CANMSG_SIGNATURE_START    0xAA55A55A
#define CANMSG_SIGNATURE_RESET    0x12345678

typedef struct
{
  union {
    struct {
      uint32_t RCI:2;           /* Redundancy channel ID */
      uint32_t OC:6;            /* Op-code */
      uint32_t SID:4;           /* Servo ID */
      uint32_t SP1:1;           /* Spare, reserved, always 1 */
      uint32_t SRC:2;           /* Source: FCC1..3/Servo */
      uint32_t SP2:1;           /* Spare, reserved, always 1 */
      uint32_t PVT:1;           /* Private data, always 0 */
      uint32_t LCL:1;           /* Local bus only, always 1 */
      uint32_t FSB:1;           /* Functional status bit, always 0 */
      uint32_t SFID:7;          /* Source FID */
      uint32_t LCC:3;           /* Logical communication channel */
    } bit;
    uint32_t value;
  };
} CANBUS_IdType;

bool CANBUS_SetPositionMeteor(float pos);
bool CANBUS_GetPositionMeteor(float *pos);
bool CANBUS_ReadByteMeteor(uint16_t addr, uint8_t *value);
bool CANBUS_WriteByteMeteor(uint16_t addr, uint8_t value);
bool CANBUS_StartBLMeteor(void);
bool CANBUS_GoToAppMeteor(void);

#endif
