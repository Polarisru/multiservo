#include "driver_adc.h"
#include "driver_gpio.h"

/** \brief Setup ADC module
 *
 * \param [in] ref Reference source
 * \param [in] resolution ADC resolution
 * \return Nothing
 *
 */
void ADC_Init(Adc *channel, uint8_t ref, uint8_t resolution)
{
  /**< Configure clock and power */
  if (channel == ADC0)
  {
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_ADC0;
    GCLK->PCHCTRL[ADC0_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  if (channel == ADC1)
  {
    MCLK->APBCMASK.reg |= MCLK_APBCMASK_ADC1;
    GCLK->PCHCTRL[ADC1_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  {
    return;
  }
  /**< Reset ADC registers */
  channel->CTRLA.bit.SWRST = 1;
  /**< Set reference */
  channel->REFCTRL.bit.REFSEL = ref;
  /**< Set freerun mode */
  channel->CTRLC.bit.FREERUN = 1;
  /**< Set resolution */
  channel->CTRLC.bit.RESSEL = resolution;
  /**< Set GND as negative ADC pin */
  channel->INPUTCTRL.bit.MUXNEG = 0x18;
  /**< Adjusting result */
  channel->AVGCTRL.reg = ADC_AVGCTRL_ADJRES(4) | ADC_AVGCTRL_SAMPLENUM_32;
  /**< Set prescaler value */
  channel->CTRLB.bit.PRESCALER = ADC_CTRLB_PRESCALER_DIV32_Val;
}

/** \brief Set active ADC channel
 *
 * \param [in] channel Number of the channel
 * \return Nothing
 *
 */
void ADC_SetChannel(Adc *channel, uint8_t input)
{
  /**< Disable ADC */
  channel->CTRLA.bit.ENABLE = 0;
  while (channel->SYNCBUSY.reg & (ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE));
  /**< Set channel */
  channel->INPUTCTRL.bit.MUXPOS = input;
  /**< Enable ADC */
  channel->CTRLA.bit.ENABLE = 1;
  while (channel->SYNCBUSY.reg & (ADC_SYNCBUSY_SWRST | ADC_SYNCBUSY_ENABLE));
  /**< Reset ready flag */
  channel->INTFLAG.reg |= ADC_INTFLAG_RESRDY;
  /**< Start conversion */
  channel->SWTRIG.bit.START = 1;
}

/** \brief Get result status
 *
 * \return True if ADC result is ready
 *
 */
bool ADC_IsReady(Adc *channel)
{
  return (channel->INTFLAG.bit.RESRDY == 1);
}

/** \brief Get conversion result
 *
 * \return Result as uint16_t
 *
 */
uint16_t ADC_GetResult(Adc *channel)
{
  return channel->RESULT.reg;
}
