#ifndef DRV_ADC_H
#define DRV_ADC_H

#include "defines.h"

void ADC_Init(uint8_t ref, uint8_t resolution);
void ADC_ConfigPins(void);
void ADC_SetChannel(uint8_t channel);
uint16_t ADC_GetResult(void);

#endif
