#ifndef UTILS_H
#define UTILS_H

#include "defines.h"

const char *UTILS_Int2Bin(uint32_t value, uint8_t len);
uint16_t UTILS_Median3Filter(uint16_t a, uint16_t b, uint16_t c);
char *UTILS_StrUpr(char *s);
char *UTILS_ToHex(uint8_t byte);

#endif
