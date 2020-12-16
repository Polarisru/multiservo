#include "driver_temp.h"
#include "tsens.h"

/** \brief Initialize internal temperature sensor
 *
 * \return Nothing
 *
 */
void TEMP_Init(void)
{
	MCLK->APBAMASK.reg |= MCLK_APBAMASK_TSENS;
  GCLK->PCHCTRL[TSENS_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  TSENS->CTRLA.reg = TSENS_CTRLA_SWRST;
	while (TSENS->SYNCBUSY.reg & TSENS_SYNCBUSY_SWRST);

	TSENS->CAL.reg = TSENS_CAL_TCAL(CONF_TSENS_CAL_TCAL) | TSENS_CAL_FCAL(CONF_TSENS_CAL_FCAL);
	TSENS->GAIN.reg = CONF_TSENS_CAL_GAIN;
	TSENS->OFFSET.reg = CONF_TSENS_CAL_OFFSET;

  TSENS->CTRLC.reg = TSENS_CTRLC_FREERUN | TSENS_CTRLC_WINMODE(TSENS_CTRLC_WINMODE_DISABLE_Val);
  TSENS->CTRLA.reg = TSENS_CTRLA_ENABLE;
  TSENS->CTRLB.reg = TSENS_CTRLB_START;
}

/** \brief Get temperature of the internal sensor
 *
 * \return Temperature in °C as float
 *
 */
float TEMP_GetValue(void)
{
  int32_t value = TSENS->VALUE.bit.VALUE;

  return (float)value / 100;
}
