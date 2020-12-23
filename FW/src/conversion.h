#ifndef _CONVERSION_H
#define _CONVERSION_H

#include "defines.h"

#define CONVERSION_IREF_RESISTANCE    0.02f
#define CONVERSION_I_AMPLIF           50

#define CONVERSION_TEMP_COEFF_B       0.500f
#define CONVERSION_TEMP_COEFF_K       0.010f

float CONVERSION_GetVoltage(void);
float CONVERSION_GetSupplyVoltage(void);
float CONVERSION_GetCurrent(uint8_t type);
float CONVERSION_GetCurrentSpike(void);
void CONVERSION_Task(void *pvParameters);

#endif
