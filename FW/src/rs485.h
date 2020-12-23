#ifndef RS485_H
#define RS485_H

#include "defines.h"

#define RS485_PAKET_SIZE    6
#define RS485_CRC_LEN       sizeof(uint16_t)
#define RS485_OFFS_CMD      0
#define RS485_OFFS_ID       1
#define RS485_OFFS_DATA     2
#define RS485_OFFS_CRC      (RS485_PAKET_SIZE - RS485_CRC_LEN)

void RS485_SetBaudrate(uint32_t baudrate);
void RS485_Disable(void);
bool RS485_Transfer(uint8_t *tx_data, uint8_t reply, uint8_t *rx_data, uint8_t timeout);
void RS485_Configuration(void);

#endif

