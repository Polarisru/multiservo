#include "driver_timer.h"

#define TIMER_NUM         TC1
#define TIMER_MCLK        MCLK_APBCMASK_TC1
#define TIMER_PRESCALER   TC_CTRLA_PRESCALER_DIV1024_Val
#define TIMER_GCLK_FREQ   48000000UL
#define TIMER_GCLK_ID     TC1_GCLK_ID
#define TIMER_FREQ        (TIMER_GCLK_FREQ / 1024)
#define TIMER_1MS_TICK    (TIMER_FREQ / 1000)

//void TC1_Handler(void)
//{
//  if ((TIMER_NUM->COUNT8.INTFLAG.reg & TC_INTFLAG_OVF) >> TC_INTFLAG_OVF_Pos)
//  {
//    TIMER_NUM->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
//    //PORT_IOBUS->Group[GPIO_PORTA].OUTTGL.reg = (1UL << 8);
//  }
//}

/** \brief Initialize timer module
 *
 * \return Nothing
 *
 */
void TIMER_Init(void)
{
  /* power on the device */
  MCLK->APBCMASK.reg |= TIMER_MCLK;
  GCLK->PCHCTRL[TIMER_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  TIMER_NUM->COUNT16.CTRLA.reg = TC_CTRLA_SWRST;
  while (TIMER_NUM->COUNT16.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);
  TIMER_NUM->COUNT16.CTRLA.reg |= (TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | TC_CTRLA_MODE_COUNT16;
  TIMER_NUM->COUNT16.CTRLBSET.bit.DIR = 1;
//  TIMER_NUM->COUNT8.EVCTRL.bit.OVFEO = 1;
//  NVIC_ClearPendingIRQ(TC1_IRQn);
//  TIMER_NUM->COUNT8.INTENSET.reg = TC_INTENSET_OVF;
//  NVIC_EnableIRQ(TC1_IRQn);
}

/** \brief Start timer with preset timeout in milliseconds
 *
 * \param [in] timeout Timeout value in ms
 * \return Nothing
 *
 */
void TIMER_StartMs(uint16_t timeout)
{
  uint16_t temp = timeout * TIMER_1MS_TICK;
  TIMER_NUM->COUNT16.COUNT.reg = temp;
  TIMER_NUM->COUNT16.CTRLBSET.bit.ONESHOT = 1;
  TIMER_NUM->COUNT16.CTRLA.reg |= (TC_CTRLA_ENABLE);
  while (TIMER_NUM->COUNT16.SYNCBUSY.reg & TC_SYNCBUSY_ENABLE);
}

/** \brief Stop timer
 *
 * \return Nothing
 *
 */
void TIMER_Stop(void)
{
  TIMER_NUM->COUNT16.INTFLAG.reg = TC_INTFLAG_OVF;
  TIMER_NUM->COUNT16.CTRLA.reg &= (~TC_CTRLA_ENABLE);
  while (TIMER_NUM->COUNT16.SYNCBUSY.reg & TC_SYNCBUSY_ENABLE);
}

/** \brief Check if timeout occurred
 *
 * \return True if timeout occurred
 *
 */
bool TIMER_IsTimeout(void)
{
  if (TIMER_NUM->COUNT16.INTFLAG.bit.OVF)
    return true;
  return false;
}

/** \brief Do delay in milliseconds
 *
 * \param [in] msec Delay value in msec
 * \return Nothing
 *
 */
void TIMER_DelayMs(uint16_t msec)
{
  TIMER_StartMs(msec);
  while (!TIMER_IsTimeout());
  TIMER_Stop();
}

/** \brief Generate timer pulses with usec frequency
 *
 * \param [in] usec Time in microseconds
 * \return Nothing
 *
 */
//void TIMER_GenerateUs(uint16_t usec)
//{
//  uint8_t temp = (uint8_t)((uint32_t)usec * TIMER_1MS_TICK / 1000);
//  TIMER_NUM->COUNT8.COUNT.reg = 0;
//  TIMER_NUM->COUNT8.PER.reg = temp;
//  TIMER_NUM->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
//  TIMER_NUM->COUNT8.CTRLBSET.bit.ONESHOT = 0;
//  TIMER_NUM->COUNT8.CTRLA.reg |= (TC_CTRLA_ENABLE);
//  while (TIMER_NUM->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_ENABLE);
//}
