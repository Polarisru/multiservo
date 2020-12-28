#ifndef DRV_UART_H
#define DRV_UART_H

#include "defines.h"

void UART_Init(Sercom *channel, uint8_t pinRXPO, uint8_t pinTXPO, uint32_t baud, bool stop2);
void UART_DeInit(Sercom *channel);
void UART_SetBaudrate(Sercom *channel, uint32_t baud);
void UART_SendByte(Sercom *channel, uint8_t data);
bool UART_HaveData(Sercom *channel);
uint8_t UART_GetByte(Sercom *channel);

#endif
