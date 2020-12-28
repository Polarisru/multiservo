#ifndef UART_H
#define UART_H

#include "defines.h"

void RS232_SendChar(uint8_t c);
uint8_t RS232_GetChar(void);
bool RS232_HaveData(void);
void RS232_Disable(void);
void RS232_Configuration(void);

#endif
