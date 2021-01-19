#include "analog.h"

#define ANALOG_BUFF_LEN     1024

#define ANALOG_DMA_TIMER        TC3
#define ANALOG_DMA_TIMER_GCLK   TC3_GCLK_ID
#define ANALOG_DMA_TIMER_MCLK   MCLK_APBCMASK_TC3
#define ANALOG_DMA_OUT_ID       TC3_DMAC_ID_OVF
#define ANALOG_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV64_Val
#define ANALOG_TIMER_VALUE      (CONF_GCLK_TC0_FREQUENCY / 1000000UL * ANALOG_BIT_LEN / 64)

uint8_t ANALOG_Buff[ANALOG_BUFF_LEN];

const TAnalogChannel ANALOG_Channels[ADC_CHANNEL_NUM] = {
  {GPIO_PORTA, 6, MUX_PA06B_ADC0_AIN6, ADC_INPUTCTRL_MUXPOS_AIN6_Val},
  {GPIO_PORTA, 7, MUX_PA07B_ADC0_AIN7, ADC_INPUTCTRL_MUXPOS_AIN7_Val},
  {GPIO_PORTA, 9, MUX_PA09B_ADC1_AIN11, ADC_INPUTCTRL_MUXPOS_AIN11_Val}
};

uint16_t ANALOG_GetValueADC0(uint8_t channel, uint8_t type)
{
  ADC_SetChannel(ADC0, ANALOG_Channels[channel].channel);
  while (ADC_IsReady(ADC0) == false);
  return ADC_GetResult(ADC0);
}

uint16_t ANALOG_GetValueADC1(uint8_t channel, uint8_t type)
{
  return ADC_GetResult(ADC1);
}

void ANALOG_StartDMA(void)
{
  DMA_StartChannel(DMA_CHANNEL_ADC_I);
}

uint16_t ANALOG_GetSpikeValue(void)
{
  return 0;
}

void ANALOG_Configuration(void)
{
  uint8_t i;
  TDmaSettings DmaSett;

  /**< Setup analog pins */
  for (i = 0; i < ADC_CHANNEL_NUM; i++)
  {
    GPIO_SetDir(ANALOG_Channels[i].port, ANALOG_Channels[i].pin, false);
    GPIO_SetFunction(ANALOG_Channels[i].port, ANALOG_Channels[i].pin, ANALOG_Channels[i].pin_mode);
  }

  /**< Setup ADC */
  VREF_Init(SUPC_VREF_SEL_4V096_Val);
	ADC_Init(ADC0, ADC_REFCTRL_REFSEL_INTREF_Val, ADC_CTRLC_RESSEL_12BIT_Val);
	ADC_Init(ADC1, ADC_REFCTRL_REFSEL_INTREF_Val, ADC_CTRLC_RESSEL_12BIT_Val);
  ADC_SetChannel(ADC1, ANALOG_Channels[ADC_CHANNEL_I].channel);

  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
  DmaSett.trig_src = ANALOG_DMA_OUT_ID;
  DmaSett.src_addr = (void*)&ADC1->RESULT.reg;
  DmaSett.dst_addr = (void*)ANALOG_Buff;
  DmaSett.src_inc = false;
  DmaSett.dst_inc = true;
  DmaSett.len = ANALOG_BUFF_LEN;
  DmaSett.priority = 0;
  DMA_SetupChannel(DMA_CHANNEL_ADC_I, &DmaSett);
}
