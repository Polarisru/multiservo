#include "driver_gpio.h"


/** \brief Initialize GPIO module
 *
 * \return Nothing
 *
 */
void GPIO_Init(void)
{

}

/** \brief Set pin direction
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \param [in] dir Direction: true - output, false - input
 * \return Nothing
 *
 */
void GPIO_SetDir(uint8_t port, uint8_t pin, bool dir)
{
  if (pin > GPIO_MAX_PIN)
    return;
  if (dir == true)
  {
    PORT->Group[port].DIRSET.reg = (1UL << pin);
  } else
  {
    PORT->Group[port].DIRCLR.reg = (1UL << pin);
    PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_INEN;
  }
}

/** \brief Set pin function
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \param [in] mode Pin mode (alternate function)
 * \return Nothing
 *
 */
void GPIO_SetFunction(uint8_t port, uint8_t pin, uint8_t mode)
{
  if (pin > GPIO_MAX_PIN)
    return;
  if (mode >= GPIO_PIN_FUNC_OFF)
  {
    PORT->Group[port].PINCFG[pin].reg &= ~PORT_PINCFG_PMUXEN;
  } else
  {
    PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PMUXEN;
    if (pin % 2)
      PORT->Group[port].PMUX[pin >> 1].reg |= (mode << 4);
    else
      PORT->Group[port].PMUX[pin >> 1].reg = mode;
  }
}

/** \brief Set pull-up or pull-down for pin
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \param [in] mode Mode for pin (no pulls/pull-up/pull-down)
 * \return Nothing
 *
 */
void GPIO_SetPullMode(uint8_t port, uint8_t pin, uint8_t mode)
{
  if (pin > GPIO_MAX_PIN)
    return;
  switch (mode)
  {
    case GPIO_PULLMODE_NONE:
      PORT->Group[port].PINCFG[pin].reg &= ~PORT_PINCFG_PULLEN;
      break;
    case GPIO_PULLMODE_UP:
      PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PULLEN;
      PORT->Group[port].OUTSET.reg = (1UL << pin);
      break;
    case GPIO_PULLMODE_DOWN:
      PORT->Group[port].PINCFG[pin].reg |= PORT_PINCFG_PULLEN;
      PORT->Group[port].OUTCLR.reg = (1UL << pin);
      break;
  }
}

/** \brief Set pin high
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \return Nothing
 *
 */
void GPIO_SetPin(uint8_t port, uint8_t pin)
{
  //if (pin > GPIO_MAX_PIN)
  //  return;
  PORT->Group[port].OUTSET.reg = (1UL << pin);
}

/** \brief Set pin low
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \return Nothing
 *
 */
void GPIO_ClearPin(uint8_t port, uint8_t pin)
{
  //if (pin > GPIO_MAX_PIN)
  //  return;
  PORT->Group[port].OUTCLR.reg = (1UL << pin);
}

/** \brief Toggle pin
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \return void
 *
 */
void GPIO_TogglePin(uint8_t port, uint8_t pin)
{
  //if (pin > GPIO_MAX_PIN)
  //  return;
	PORT->Group[port].OUTTGL.reg = (1UL << pin);
}

/** \brief Get pin value
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \return True if high, false otherwise
 *
 */
bool GPIO_GetPin(uint8_t port, uint8_t pin)
{
  //if (pin > GPIO_MAX_PIN)
  //  return false;
  return (PORT->Group[port].IN.reg & (1UL << pin));
}

/** \brief Get pin output
 *
 * \param [in] port Port number
 * \param [in] pin Pin number
 * \return True if high, false otherwise
 *
 */
bool GPIO_GetOutputPin(uint8_t port, uint8_t pin)
{
  //if (pin > GPIO_MAX_PIN)
  //  return false;
  return (PORT->Group[port].OUT.reg & (1UL << pin));
}
