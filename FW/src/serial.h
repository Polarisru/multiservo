#ifndef SERIAL_H
#define SERIAL_H

#include "defines.h"

void SERIAL_Enable(void);
void SERIAL_Disable(void);
void SERIAL_SetBaudrate(uint32_t baudrate);
void SERIAL_Send(uint8_t *data, uint16_t len);
bool SERIAL_Receive(uint8_t *data, uint16_t len, uint16_t timeout);
void SERIAL_Configuration(void);

#endif
