#ifndef CRC8_H
#define CRC8_H

#include "defines.h"

#define CRC8_INIT   0xFF

uint8_t CRC8_AddWord(uint8_t crc, uint16_t data);
uint8_t CRC8_Calc(uint8_t *data, uint8_t len);

#endif

