#ifndef PWMOUT_H
#define PWMOUT_H

#include "defines.h"

void PWMOUT_Enable(void);
void PWMOUT_Disable(void);
void PWMOUT_EnableUART(uint32_t baudrate);
void PWMOUT_SetValue(float value);
void PWMOUT_Configuration(void);

#endif
