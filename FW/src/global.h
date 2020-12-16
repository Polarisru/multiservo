#ifndef GLOBAL_H
#define GLOBAL_H

#include "defines.h"

/**< List of global variables */
uint8_t GLOBAL_mode;
int16_t GLOBAL_TargetPos;
int16_t GLOBAL_RealPos;
uint16_t GLOBAL_MagnetValue;
uint8_t GLOBAL_Power;
uint32_t GLOBAL_WorkingTime;
volatile bool GLOBAL_GotNMI;
uint16_t GLOBAL_ValATA6832;

/**< EEPROM variables with default values */
uint8_t  EE_Id;
uint16_t EE_CanId;
uint8_t  EE_Kd;
uint8_t  EE_Kp;
uint8_t  EE_PWM_Min;
uint8_t  EE_Ko;
uint8_t  EE_PWM_Max;
uint8_t  EE_Signal_DB;
uint16_t EE_Sensor_DB;
uint16_t EE_Min_us;
uint16_t EE_Max_us;
uint8_t  EE_Options;
uint8_t  EE_Expansion;
uint8_t  EE_PWM_Safe;
uint16_t EE_FailsafePos;
uint8_t  EE_FailsafeTime;
int16_t  EE_Neutral;
uint8_t  EE_Saver_Time;
uint8_t  EE_Saver_Frame;
uint8_t  EE_CCW_Scaling;
uint8_t  EE_CW_Scaling;
uint16_t EE_Point_cutoff;
uint16_t EE_Point_idle;
uint16_t EE_Point_full;
uint16_t EE_Mag_idle;
uint16_t EE_Mag_full;
uint16_t EE_CO_FB_Voltage;
uint16_t EE_FT_FB_Voltage;
char     EE_Name[SERVO_NAME_LEN];

/**< Tasks IDs */
TaskHandle_t taskHall;

/**< Semaphores for RTOS */
SemaphoreHandle_t xSemaphoreEEPROM;
SemaphoreHandle_t xSemaphoreSPI;

#endif
