#ifndef RS232_H
#define RS232_H

#include "defines.h"

#define RS232_BUFFER_SIZE     256

char RS232_RxBuffer[RS232_BUFFER_SIZE];

void RS232_SendData(uint8_t *data, uint8_t len);
void RS232_SetBaudrate(uint32_t baudrate);
void RS232_Configuration(void);

#endif
