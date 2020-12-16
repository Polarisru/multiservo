#ifndef CRC16_H
#define CRC16_H

#include "defines.h"

uint16_t CRC16_CalcKearfott(uint8_t *input, uint8_t len);
uint16_t CRC16_CalcCCITT(uint8_t *data, uint16_t len);

#endif
