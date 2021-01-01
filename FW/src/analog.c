#include "driver_adc.h"
#include "driver_gpio.h"
#include "driver_vref.h"
#include "analog.h"

const TAnalogChannel ANALOG_Channels[ADC_CHANNEL_NUM] = {
  {GPIO_PORTA, 6, MUX_PA06B_ADC0_AIN6, ADC_INPUTCTRL_MUXPOS_AIN6_Val},
  {GPIO_PORTA, 7, MUX_PA07B_ADC0_AIN7, ADC_INPUTCTRL_MUXPOS_AIN7_Val},
  {GPIO_PORTA, 9, MUX_PA09B_ADC0_AIN9, ADC_INPUTCTRL_MUXPOS_AIN9_Val}
};

uint16_t ANALOG_GetValue(uint8_t channel, uint8_t type)
{
  ADC_SetChannel(ANALOG_Channels[channel].channel);
  while (ADC_IsReady() == false);
  return ADC_GetResult();
}

uint16_t ANALOG_GetSpikeValue(void)
{
  return 0;
}

void ANALOG_Configuration(void)
{
  uint8_t i;

  /**< Setup analog pins */
  for (i = 0; i < ADC_CHANNEL_NUM; i++)
  {
    GPIO_SetDir(ANALOG_Channels[i].port, ANALOG_Channels[i].pin, false);
    GPIO_SetFunction(ANALOG_Channels[i].port, ANALOG_Channels[i].pin, ANALOG_Channels[i].pin_mode);
  }

  /**< Setup ADC */
  VREF_Init(SUPC_VREF_SEL_4V096_Val);
	ADC_Init(ADC_REFCTRL_REFSEL_INTREF_Val, ADC_CTRLC_RESSEL_12BIT_Val);
  //ADC_SetChannel(MONITOR_ADC_Channels[MONITOR_ADC_CH_CURRENT].channel);
}
