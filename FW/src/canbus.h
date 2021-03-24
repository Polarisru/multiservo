#ifndef CANBUS_H
#define CANBUS_H

#include "defines.h"
#include "driver_can.h"

#define CANBUS_MAX_RETRIES        3
#define CANBUS_MAX_LEN            32

/**< CAN reply timeout in ms */
#define CANBUS_TIMEOUT            20

typedef struct
{
  uint32_t  id;
  uint8_t   data[CANBUS_MAX_LEN];
  uint8_t   len;
  enum can_format fmt;
} TCanMsg;

bool CANBUS_SendMessage(uint32_t id, uint8_t *data, uint8_t datalen, bool ext, bool fd, bool brs);
bool CANBUS_ReceiveMessage(TCanMsg *msg);
bool CANBUS_SetBaudrate(uint32_t nominal_baudrate, uint32_t data_baudrate);
void CANBUS_Disable(void);
void CANBUS_Configuration(void);

#endif
