#include "driver_gpio.h"
#include "outputs.h"

/** \brief Initialize outputs module
 *
 * \return Nothing
 *
 */
void OUTPUTS_Configuration(void)
{
  /**< Configure output pins */
  GPIO_ClearPin(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN);
  GPIO_SetDir(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN, true);
  GPIO_SetFunction(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN, GPIO_PIN_FUNC_OFF);
  GPIO_ClearPin(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN);
  GPIO_SetDir(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN, true);
  GPIO_SetFunction(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN, GPIO_PIN_FUNC_OFF);
  GPIO_ClearPin(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN);
  GPIO_SetDir(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN, true);
  GPIO_SetFunction(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN, GPIO_PIN_FUNC_OFF);
  GPIO_ClearPin(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN);
  GPIO_SetDir(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN, true);
  GPIO_SetFunction(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN, GPIO_PIN_FUNC_OFF);
  GPIO_ClearPin(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN);
  GPIO_SetDir(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN, true);
  GPIO_SetFunction(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN, GPIO_PIN_FUNC_OFF);
}

/** \brief Switch output on/off
 *
 * \param [in] num Output number
 * \param [in] on Switch output on or off (OUTPUTS_SWITCH_ON/OUTPUTS_SWITCH_OFF)
 * \return
 *
 */
void OUTPUTS_Switch(uint8_t num, uint8_t on)
{
  switch (num)
  {
    case OUTPUTS_LED1:
      if (on == OUTPUTS_SWITCH_OFF)
        GPIO_ClearPin(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN);
      else
        GPIO_SetPin(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN);
      break;
    case OUTPUTS_LED2:
      if (on == OUTPUTS_SWITCH_OFF)
        GPIO_ClearPin(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN);
      else
        GPIO_SetPin(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN);
      break;
    case OUTPUTS_SERVO:
      if (on == OUTPUTS_SWITCH_OFF)
        GPIO_ClearPin(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN);
      else
        GPIO_SetPin(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN);
      break;
    case OUTPUTS_SBUSPOL:
      if (on == OUTPUTS_SWITCH_OFF)
        GPIO_ClearPin(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN);
      else
        GPIO_SetPin(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN);
      break;
    case OUTPUTS_SBUSTE:
      if (on == OUTPUTS_SWITCH_OFF)
        GPIO_ClearPin(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN);
      else
        GPIO_SetPin(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN);
      break;
    default:
      break;
  }
}

/** \brief Toggle output
 *
 * \param [in] num Output number
 * \return Nothing
 *
 */
void OUTPUTS_Toggle(uint8_t num)
{
  switch (num)
  {
    case OUTPUTS_LED1:
      GPIO_TogglePin(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN);
      break;
    case OUTPUTS_LED2:
      GPIO_TogglePin(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN);
      break;
    case OUTPUTS_SERVO:
      GPIO_TogglePin(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN);
      break;
    case OUTPUTS_SBUSPOL:
      GPIO_TogglePin(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN);
      break;
    case OUTPUTS_SBUSTE:
      GPIO_TogglePin(OUTPUTS_SBUSTE_GPIO, OUTPUTS_SBUSTE_PIN);
      break;
    default:
      break;
  }
}

/** \brief Check if output pin is active (has high level)
 *
 * \param [in] num Output number
 * \return True if pin is active
 *
 */
bool OUTPUTS_IsActive(uint8_t num)
{
  switch (num)
  {
    case OUTPUTS_LED1:
      return GPIO_GetOutputPin(OUTPUTS_LED1_GPIO, OUTPUTS_LED1_PIN);
    case OUTPUTS_LED2:
      return GPIO_GetOutputPin(OUTPUTS_LED2_GPIO, OUTPUTS_LED2_PIN);
    case OUTPUTS_SERVO:
      return GPIO_GetOutputPin(OUTPUTS_SERVO_GPIO, OUTPUTS_SERVO_PIN);
    case OUTPUTS_SBUSPOL:
      return GPIO_GetOutputPin(OUTPUTS_SBUSPOL_GPIO, OUTPUTS_SBUSPOL_PIN);
    default:
      return false;
  }
}
