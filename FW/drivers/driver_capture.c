#include "driver_capture.h"

// Event generator, determines event generator for channel
// <0x0=>No event generator
// <0x1=>XOSC Clock Failure
// <0x2=>XOSC32K Clock Failure
// <0x3=>RTC compare 0 or alarm 0
// <0x4=>RTC compare 1
// <0x5=>RTC overflow
// <0x6=>RTC period 0
// <0x7=>RTC period 1
// <0x8=>RTC period 2
// <0x9=>RTC period 3
// <0xA=>RTC period 4
// <0xB=>RTC period 5
// <0xC=>RTC period 6
// <0xD=>RTC period 7
// <0xE=>EIC external interrupt 0
// <0xF=>EIC external interrupt 1
// <0x10=>EIC external interrupt 2
// <0x11=>EIC external interrupt 3
// <0x12=>EIC external interrupt 4
// <0x13=>EIC external interrupt 5
// <0x14=>EIC external interrupt 6
// <0x15=>EIC external interrupt 7
// <0x16=>EIC external interrupt 8
// <0x17=>EIC external interrupt 9
// <0x18=>EIC external interrupt 10
// <0x19=>EIC external interrupt 11
// <0x1A=>EIC external interrupt 12
// <0x1B=>EIC external interrupt 13
// <0x1C=>EIC external interrupt 14
// <0x1D=>EIC external interrupt 15
// <0x1E=>TSENS Window Monitor
// <0x1F=>DMAC channel 0
// <0x20=>DMAC channel 1
// <0x21=>DMAC channel 2
// <0x22=>DMAC channel 3
// <0x23=>TCC0 overflow
// <0x24=>TCC0 trig
// <0x25=>TCC0 counter
// <0x26=>TCC0 match/capture 0
// <0x27=>TCC0 match/capture 1
// <0x28=>TCC0 match/capture 2
// <0x29=>TCC0 match/capture 3
// <0x2A=>TCC1 overflow
// <0x2B=>TCC1 trig
// <0x2C=>TCC1 counter
// <0x2D=>TCC1 match/capture 0
// <0x2E=>TCC1 match/capture 1
// <0x2F=>TCC2 overflow
// <0x30=>TCC2 trig
// <0x31=>TCC2 counter
// <0x32=>TCC2 match/capture 0
// <0x33=>TCC2 match/capture 1
// <0x34=>TC0 overflow
// <0x35=>TC0 match/capture 0
// <0x36=>TC0 match/capture 1
// <0x37=>TC1 overflow
// <0x38=>TC1 match/capture 0
// <0x39=>TC1 match/capture 1
// <0x3A=>TC2 overflow
// <0x3B=>TC2 match/capture 0
// <0x3C=>TC2 match/capture 1
// <0x3D=>TC3 overflow
// <0x3E=>TC3 match/capture 0
// <0x3F=>TC3 match/capture 1
// <0x40=>TC4 overflow
// <0x41=>TC4 match/capture 0
// <0x42=>TC4 match/capture 1
// <0x43=>ADC0 result ready
// <0x44=>ADC0 window monitor
// <0x45=>ADC1 result ready
// <0x46=>ADC1 window monitor
// <0x47=>SDADC result ready
// <0x48=>SDADC window monitor
// <0x49=>AC comparator 0
// <0x4A=>AC comparator 1
// <0x4B=>AC comparator 2
// <0x4C=>AC comparator 3
// <0x4D=>AC window 0
// <0x4E=>AC window 1
// <0x4F=>DAC data buffer empty
// <0x50=>PTC end of conversion
// <0x51=>PTC window comparator
// <0x52=>CCL LUT output 0
// <0x53=>CCL LUT output 1
// <0x54=>CCL LUT output 2
// <0x55=>CCL LUT output 3
// <0x56=>PAC access error
// <0x58=>TC5 overflow
// <0x59=>TC5 match/capture 0
// <0x5A=>TC5 match/capture 1
// <0x5B=>TC6 overflow
// <0x5C=>TC6 match/capture 0
// <0x5D=>TC6 match/capture 1
// <0x5E=>TC7 overflow
// <0x5F=>TC7 match/capture 0
// <0x60=>TC7 match/capture 1

#define CAPTURE_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV8_Val
#define CAPTURE_TIMER_DIVIDER    (CONF_CPU_FREQUENCY / 8 / 1000000UL)

volatile uint32_t pulse_width, period;

void TC0_Handler(void)
{
  if ((TC0->COUNT16.INTFLAG.reg & TC_INTFLAG_OVF) >> TC_INTFLAG_OVF_Pos)
  {
    TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;
  }
  if ((TC0->COUNT16.INTFLAG.reg & TC_INTFLAG_ERR) >> TC_INTFLAG_ERR_Pos)
  {
    TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_ERR;
  }
  if ((TC0->COUNT16.INTFLAG.reg & TC_INTFLAG_MC1) >> TC_INTFLAG_MC1_Pos)
  {
    TC0->COUNT16.INTFLAG.reg = TC_INTFLAG_MC1;
    while (TC0->COUNT32.SYNCBUSY.bit.CC0);
    pulse_width = TC0->COUNT32.CC[0].reg / CAPTURE_TIMER_DIVIDER;
    while (TC0->COUNT32.SYNCBUSY.bit.CC1);
    period = TC0->COUNT32.CC[1].reg / CAPTURE_TIMER_DIVIDER;
  }
}

void CAPTURE_Disable(void)
{
  NVIC_DisableIRQ(TC0_IRQn);
}

void CAPTURE_Enable(void)
{
  NVIC_ClearPendingIRQ(TC0_IRQn);
  NVIC_EnableIRQ(TC0_IRQn);
}

void CAPTURE_EnablePin(void)
{
  /**< Set pin direction to input with external interrupt */
  GPIO_SetDir(CAPTURE_PORT, CAPTURE_PIN, false);
  GPIO_SetPullMode(CAPTURE_PORT, CAPTURE_PIN, GPIO_PULLMODE_UP);
  GPIO_SetFunction(CAPTURE_PORT, CAPTURE_PIN, MUX_PA22A_EIC_EXTINT6);
}

void CAPTURE_DisablePin(void)
{
  /**< Set pin direction to output */
  GPIO_SetFunction(CAPTURE_PORT, CAPTURE_PIN, GPIO_PIN_FUNC_OFF);
  GPIO_SetDir(CAPTURE_PORT, CAPTURE_PIN, true);
  GPIO_ClearPin(CAPTURE_PORT, CAPTURE_PIN);
}

void CAPTURE_Init(void)
{
  /**< Setup power and clock for pin interrupt as event */
  GCLK->PCHCTRL[EIC_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_EIC;

  CAPTURE_EnablePin();

  /**< Set pin event generation */
  if (EIC->CTRLA.bit.ENABLE == 0)
  {
    EIC->CTRLA.reg = EIC_CTRLA_SWRST;
    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_SWRST);
  } else
  {
    EIC->CTRLA.bit.ENABLE = 0;
    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);
  }
  EIC->NMICTRL.reg |= (0 << EIC_NMICTRL_NMIFILTEN_Pos) | EIC_NMICTRL_NMISENSE(EIC_NMICTRL_NMISENSE_NONE_Val) | EIC_ASYNCH_ASYNCH(0);
  EIC->EVCTRL.reg |= (1 << 6);
  EIC->ASYNCH.reg |= (1 << 6);
  EIC->DEBOUNCEN.reg |= (1 << 6);
  EIC->DPRESCALER.reg |= 0;
  /**< Setup Ext.Pin 6 (CONFIG0) */
  EIC->CONFIG[0].reg |= (0 << EIC_CONFIG_FILTEN6_Pos) | EIC_CONFIG_SENSE6(EIC_NMICTRL_NMISENSE_HIGH_Val);
  EIC->CONFIG[1].reg |= 0;
  EIC->CTRLA.reg |= EIC_CTRLA_ENABLE;
  while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);

  /**< Setup timer for input capture */
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC0;
  GCLK->PCHCTRL[TC0_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  /**< Enable timer interrupt */
  CAPTURE_Disable();
  CAPTURE_Enable();

  //TC0->COUNT16.CTRLA.bit.SWRST = 1;   // added this recently, not checked yet!!!
  //while (TC0->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);

  TC0->COUNT16.CTRLA.reg = (1 << TC_CTRLA_CAPTEN0_Pos) | (1 << TC_CTRLA_CAPTEN1_Pos) | \
                           (CAPTURE_TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | (0x2 << TC_CTRLA_MODE_Pos);
  while (TC0->COUNT8.SYNCBUSY.reg & (TC_SYNCBUSY_SWRST | TC_SYNCBUSY_ENABLE));

  TC0->COUNT16.CTRLBSET.reg = 0;

  /* EVACT[2:0]: Event Action - Period captured in CC1, pulse width in CC0 */
  TC0->COUNT16.EVCTRL.reg = (1 << TC_EVCTRL_TCEI_Pos) | 6;

  /* Match or Capture Channel 1 Interrupt Enable: disabled */
  TC0->COUNT16.INTENSET.reg = (1 << TC_INTENSET_MC1_Pos);
  TC0->COUNT16.INTENCLR.reg = ~(1 << TC_INTENSET_MC1_Pos);

  TC0->COUNT16.CTRLA.bit.ENABLE = 1;
  while (TC0->COUNT8.SYNCBUSY.reg & (TC_SYNCBUSY_SWRST | TC_SYNCBUSY_ENABLE));

  GCLK->PCHCTRL[EVSYS_GCLK_ID_0].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_EVSYS;
  //event_system_init();
  /* Event channel 0 setup, <0x14=>EIC external interrupt 6 as source */
  EVSYS->CHANNEL[0].reg = (1 << EVSYS_CHANNEL_ONDEMAND_Pos) | (0 << EVSYS_CHANNEL_RUNSTDBY_Pos)
                       | EVSYS_CHANNEL_EDGSEL(EVSYS_CHANNEL_EDGSEL_BOTH_EDGES_Val) | EVSYS_CHANNEL_PATH(EVSYS_CHANNEL_PATH_ASYNCHRONOUS_Val)
                       | EVSYS_CHANNEL_EVGEN(0x14);
  /* <23=>TC0 as event destination, <0x1=>Channel 0 */
  EVSYS->USER[23].reg = 1;
  /* no interrupts from events required */
  //EVSYS->INTENSET.reg = 0x00000000;
  //EVSYS->INTENCLR.reg = 0xFFFFFFFF;
}

/** \brief Get pulse width
 *
 * \return
 *
 */
uint32_t CAPTURE_GetPulseWidth(void)
{
  return pulse_width;
}

/** \brief Get period length
 *
 * \return
 *
 */
uint32_t CAPTURE_GetPeriod(void)
{
  return period;
}
