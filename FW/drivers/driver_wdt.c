#include "driver_wdt.h"
#include "wdt.h"

// PER configuration
// <0x0=>8 clock cycles
// <0x1=>16 clock cycles
// <0x2=>32 clock cycles
// <0x3=>64 clock cycles
// <0x4=>128 clock cycles
// <0x5=>256 clock cycles
// <0x6=>512 clock cycles
// <0x7=>1024 clock cycles
// <0x8=>2048 clock cycles
// <0x9=>4096 clock cycles
// <0xA=>8192 clock cycles
// <0xB=>16384 clock cycles

#define CONF_WDT_PER 0x4

/** \brief Initialize watchdog
 *
 * \return Nothing
 *
 */
void WDT_Init(void)
{
	uint8_t tmp;

  MCLK->APBAMASK.reg |= MCLK_APBAMASK_WDT;
 	WDT->CTRLA.reg &= ~WDT_CTRLA_WEN;
  while (WDT->SYNCBUSY.reg & (WDT_SYNCBUSY_ENABLE | WDT_SYNCBUSY_WEN | WDT_SYNCBUSY_ALWAYSON));

	tmp = WDT->CONFIG.reg;
	tmp &= ~WDT_CONFIG_PER_Msk;
	tmp |= WDT_CONFIG_PER(CONF_WDT_PER);
	WDT->CONFIG.reg = tmp;
}

/** \brief Enable watchdog
 *
 * \return Nothing
 *
 */
void WDT_Enable(void)
{
	WDT->CTRLA.reg |= WDT_CTRLA_ENABLE;
	while (WDT->SYNCBUSY.reg & (WDT_SYNCBUSY_ENABLE | WDT_SYNCBUSY_WEN | WDT_SYNCBUSY_ALWAYSON));
}

/** \brief Disable watchdog
 *
 * \return Nothing
 *
 */
void WDT_Disable(void)
{
  WDT->CTRLA.reg &= ~WDT_CTRLA_ENABLE;
	while (WDT->SYNCBUSY.reg & (WDT_SYNCBUSY_ENABLE | WDT_SYNCBUSY_WEN | WDT_SYNCBUSY_ALWAYSON));
}

/** \brief Reset watchdog
 *
 * \return Nothing
 *
 */
void WDT_Reset(void)
{
 	WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
 	while (WDT->SYNCBUSY.reg & WDT_SYNCBUSY_CLEAR);
}
