#include "analog.h"
#include "conversion.h"
#include "global.h"
#include "outputs.h"

//static float CONVERSION_VoltageFB;

/** \brief Calculate voltage from 1:2 resistor divider
 * V = ADC_Value * VREF * 2 / 4096
 *
 * \return Voltage value as float
 */
float CONVERSION_GetVoltage(void)
{
  float voltage;

  voltage = (float)ANALOG_GetValueADC0(ADC_CHANNEL_FB, ADC_TYPE_CURRENT) * ADC_REF_VOLTAGE * 2 / ADC_MAX_VALUE;

  return voltage;

  //return CONVERSION_VoltageFB;
}

/** \brief Get feedback position
 *
 * \return Servo position in degrees
 *
 */
float CONVERSION_GetFeedbackPos(void)
{
  float flVal = CONVERSION_GetVoltage();
  return (flVal / FB_VOLTAGE_ZERO - 1) * PWM_RANGE / 2;
}

/** \brief Calculate voltage from additional resistor divider
 * V = ADC_Value * VREF * DIVIDER / 4096
 *
 * \return Voltage value as float
 */
float CONVERSION_GetSupplyVoltage(void)
{
  float voltage;

  voltage = (float)ANALOG_GetValueADC0(ADC_CHANNEL_U, ADC_TYPE_CURRENT) * ADC_REF_VOLTAGE * EE_VoltSupplyDiv/ ADC_MAX_VALUE;

  return voltage;
}

/** \brief Calculate current from INA139 (two instances)
 * We have amplification factor 25/200 and 0.01R shunt, so we should get 0.25/2.0V for 1A
 * Ih = ADC_Value * VREF / 4096 / 0.01Ohm / 25
 * Il = ADC_Value * VREF / 4096 / 0.01Ohm / 200
 *
 * \param [in] type Type of ADC value (current or maximal)
 * \return Current value in A as float
 */
float CONVERSION_GetCurrent(uint8_t type)
{
  uint16_t value;
  float current;

  if (type >= ADC_TYPE_LAST)
    return 0.0;

  value = ANALOG_GetValueADC1(ADC_CHANNEL_I, type);
  if (value > EE_CurrOffset)
    value -= EE_CurrOffset;
  else
    value = 0;

  current = (float)value * ADC_REF_VOLTAGE / ADC_MAX_VALUE / CONVERSION_IREF_RESISTANCE / CONVERSION_I_AMPLIF;

  return current;
}

/** \brief Get current spike value (very high frequency)
 *
 * \return Current value as float
 *
 */
float CONVERSION_GetCurrentSpike(void)
{
  uint16_t value;
  float current;

  value = ANALOG_GetSpikeValue();
  current = (float)value * ADC_REF_VOLTAGE / ADC_MAX_VALUE / CONVERSION_IREF_RESISTANCE / CONVERSION_I_AMPLIF;

  return current;
}
