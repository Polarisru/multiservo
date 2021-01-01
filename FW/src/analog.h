#ifndef ADC_H
#define ADC_H

#include "defines.h"

#define ADC_REF_VOLTAGE     4.096f

#define ADC_MAX_VALUE       4096

#define FB_VOLTAGE_ZERO     2.5f

enum {
  ADC_CHANNEL_U,
  ADC_CHANNEL_FB,
  ADC_CHANNEL_I,
  ADC_CHANNEL_NUM
};

typedef struct {
  uint8_t port;
  uint8_t pin;
  uint8_t pin_mode;
  uint8_t channel;
} TAnalogChannel;

uint16_t ANALOG_GetValue(uint8_t channel, uint8_t type);
uint16_t ANALOG_GetSpikeValue(void);
void ANALOG_Configuration(void);

#endif
