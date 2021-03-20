#include "outputs.h"
#include "pwmout.h"
#include "serial.h"

#define PWM_PORT          GPIO_PORTA
#define PWM_PIN           0
#define PWM_PIN_MUX       MUX_PA00E_TCC2_WO0

#define PWM_TCC_NUM       TCC2
#define PWM_CHANNEL_NUM   0

#define PWM_FREQ          100UL
#define PWM_MIN_USEC      1000UL
#define PWM_MAX_USEC      2000UL

/** \brief Enable PWM pin
 *
 * \return void Nothing
 *
 */
void PWMOUT_Enable(void)
{
  GPIO_SetFunction(PWM_PORT, PWM_PIN, PWM_PIN_MUX);
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_ON);
}

/** \brief Disable PWM pin
 *
 * \return Nothing
 *
 */
void PWMOUT_Disable(void)
{
  GPIO_SetFunction(PWM_PORT, PWM_PIN, GPIO_PIN_FUNC_OFF);
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_OFF);
}

/** \brief Enable serial connection for PWM channel
 *
 * \param [in] baudrate Baudrate for serial connection
 * \return void
 *
 */
void PWMOUT_EnableUART(uint32_t baudrate)
{
  SERIAL_SetBaudrate(baudrate);
  SERIAL_Enable();
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_OFF);
}

/** \brief Set new PWM value
 *
 * \param [in] value Angle value to set in degrees
 * \return Nothing
 *
 */
void PWMOUT_SetValue(float angle)
{
  int32_t new_val;

  new_val = (uint32_t)(angle + PWM_RANGE / 2);

  if (new_val < 0)
    new_val = 0;
  if (new_val > PWM_RANGE)
    new_val = PWM_RANGE;

  new_val = (new_val * (PWM_MAX_USEC - PWM_MIN_USEC) / PWM_RANGE + PWM_MIN_USEC) * 1000 * PWM_FREQ /1000000UL;
  PWM_Set(PWM_TCC_NUM, PWM_CHANNEL_NUM, (uint16_t)new_val);
}

/** \brief Servo PWM configuration
 *
 * \param void
 * \return void
 *
 */
void PWMOUT_Configuration(void)
{
  PWM_Init(PWM_TCC_NUM, PWM_FREQ);
}
