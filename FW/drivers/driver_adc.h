#ifndef DRV_ADC_H
#define DRV_ADC_H

#include "defines.h"

void ADC_Init(Adc *channel, uint8_t ref, uint8_t resolution);
void ADC_SetChannel(Adc *channel, uint8_t input);
bool ADC_IsReady(Adc *channel);
uint16_t ADC_GetResult(Adc *channel);

#endif
