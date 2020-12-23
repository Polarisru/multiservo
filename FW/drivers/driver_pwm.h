#ifndef DRV_PWM_H
#define DRV_PWM_H

#include "defines.h"

void PWM_Init(Tcc *timer, uint32_t freq);
void PWM_Enable(Tcc *timer);
void PWM_Disable(Tcc *timer);
void PWM_Set(Tcc *timer, uint8_t channel, uint16_t value);

#endif
