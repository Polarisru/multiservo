#ifndef ADC_H
#define ADC_H

#include "defines.h"

#define ADC_REF_VOLTAGE     4.096f

#define ADC_MAX_VALUE       4096

enum {
  ADC_CHANNEL_U,
  ADC_CHANNEL_I,
  ADC_CHANNEL_FB,
  ADC_CHANNEL_NUM
};

uint16_t ADC_GetValue(uint8_t channel, uint8_t type);
uint16_t ADC_GetSpikeValue(void);
void ADC_Configuration(void);

#endif
