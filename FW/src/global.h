#ifndef GLOBAL_H
#define GLOBAL_H

#include "defines.h"

/**< List of global variables */
uint32_t GLOBAL_Baudrate;
uint8_t  GLOBAL_ConnMode;
float GLOBAL_PeakCurrent;

/**< EEPROM variables with default values */
int16_t  EE_CurrOffset;
float    EE_VoltSupplyDiv;

TaskHandle_t xTaskComm;

EventGroupHandle_t xEventGroupCommon;

#endif
