#include "global.h"

/**< List of global variables */
uint32_t GLOBAL_Baudrate;
uint8_t  GLOBAL_ConnMode;
float GLOBAL_PeakCurrent;

/**< EEPROM variables with default values */
int16_t  EE_CurrOffset;
float    EE_VoltSupplyDiv = 7.67;

TaskHandle_t xTaskComm;
