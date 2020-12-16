#ifndef DRV_CAPTURE_H
#define DRV_CAPTURE_H

#include "defines.h"
#include "driver_gpio.h"

#define CAPTURE_PORT    GPIO_PORTA
#define CAPTURE_PIN     22

void CAPTURE_Disable(void);
void CAPTURE_Enable(void);
void CAPTURE_Init(void);
void CAPTURE_EnablePin(void);
void CAPTURE_DisablePin(void);
uint32_t CAPTURE_GetPulseWidth(void);
uint32_t CAPTURE_GetPeriod(void);

#endif
