#ifndef _CONVERSION_H
#define _CONVERSION_H

#include "defines.h"

#define CONVERSION_IREF_RESISTANCE    0.01f
#define CONVERSION_IHIGH_AMPLIF       20
#define CONVERSION_ILOW_AMPLIF        100

#define CONVERSION_TEMP_COEFF_B       0.500f
#define CONVERSION_TEMP_COEFF_K       0.010f

#define CONVERSION_UPPER_RESISTANCE   10
#define CONVERSION_HIGH_RESISTANCE    20000
#define CONVERSION_LOW_RESISTANCE     82

#define CONVERSION_RES_REF_VOLTAGE    0.885f

/**< Default max.torque for BLDC = 4.48 Ncm/A (see PDF to EC-max 40) * 5A/1V * Gain(x2) * diff DAC(x2) */
#define CONVERSION_FORCE_FACTOR       (4.48f*2*2*5)
/**< Vdac * force factor */
#define MAX_FORCE_VALUE               (2.5f * CONVERSION_FORCE_FACTOR)

float CONVERSION_GetVoltage(void);
float CONVERSION_GetSupplyVoltage(void);
float CONVERSION_GetCurrent(uint8_t type);
float CONVERSION_GetCurrentSpike(void);
void CONVERSION_DoResistance(void);
bool CONVERSION_ResistanceCalibrate(uint8_t type, uint32_t ref_res, float *result);
bool CONVERSION_GetResistance(float *resistance);
float CONVERSION_GetTemperature(void);
float CONVERSION_GetForceVoltage(float force);
void CONVERSION_Task(void *pvParameters);

#endif
