#include "driver_pwm.h"
#include "driver_gpio.h"

static uint32_t PWM_MaxValue;

void PWM_Init(Tcc *timer, uint32_t freq)
{
  uint8_t division;

  /**< Power on the device */
  if (timer == TCC0)
  {
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC0;
    GCLK->PCHCTRL[TCC0_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  if (timer == TCC1)
  {
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC1;
    GCLK->PCHCTRL[TCC1_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  if (timer == TCC2)
  {
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC2;
    GCLK->PCHCTRL[TCC2_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  {
    /**< No such timer */
    return;
  }

  PWM_MaxValue = CONF_CPU_FREQUENCY / freq;
  division = 0;
  /**< Timer value must be 16-bit, doesn't work with 24-bit, don't know why */
  while (PWM_MaxValue > 0x10000)
  {
    division++;
    PWM_MaxValue /= 2;
  }

  /**< Reset timer module */
  timer->CTRLA.reg = TCC_CTRLA_SWRST;
  while (timer->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST);
  timer->CTRLBCLR.reg = TCC_CTRLBCLR_DIR;     /* count up */
  while (timer->SYNCBUSY.reg & TCC_SYNCBUSY_CTRLB);

  /**< configure the TCC device */
  timer->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(division));
  /**< Select the waveform generation mode -> normal PWM */
  timer->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
  while (timer->SYNCBUSY.reg & TCC_SYNCBUSY_WAVE);
  timer->PER.reg = PWM_MaxValue - 1;
  timer->COUNT.reg = 0;
  /**< Start timer */
  timer->CTRLA.reg |= (TCC_CTRLA_ENABLE);
  while (timer->SYNCBUSY.reg & (TCC_SYNCBUSY_SWRST | TCC_SYNCBUSY_ENABLE));
}

/** \brief Enable PWM channel
 *
 * \param [in] channel Number of the channel
 * \return Nothing
 *
 */
void PWM_Enable(Tcc *timer)
{
  timer->CTRLA.reg |= TCC_CTRLA_ENABLE;
  while (timer->SYNCBUSY.reg & TCC_SYNCBUSY_ENABLE);
}

/** \brief Disable PWM channel
 *
 * \param [in] channel Number of the channel
 * \return Nothing
 *
 */
void PWM_Disable(Tcc *timer)
{
  timer->CTRLA.reg &= (~TCC_CTRLA_ENABLE);
  while (timer->SYNCBUSY.reg & TCC_SYNCBUSY_ENABLE);
}

/** \brief Set PWM value
 *
 * \param [in] value New PWM value to set
 * \return Nothing
 *
 */
void PWM_Set(Tcc *timer, uint8_t channel, uint16_t value)
{
  timer->CC[channel].reg = (uint32_t)value * PWM_MaxValue / 1000;
}
