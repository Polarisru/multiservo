#ifndef RS485_H
#define RS485_H

#include "defines.h"

void RS485_SetBaudrate(uint32_t baudrate);
void RS485_Configuration(void);
void RS485_SwitchTermination(bool on);
void RS485_SendPacket(uint8_t *data, uint8_t len);
void RS485_Disable(void);
void RS485_Task(void *pParameters);

#endif

