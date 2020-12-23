#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "defines.h"

enum
{
  OUTPUTS_LED1,
  OUTPUTS_LED2,
  OUTPUTS_SERVO,
  OUTPUTS_SBUSPOL,
  OUTPUTS_SBUSTE,
  OUTPUTS_LAST
};

enum
{
	OUTPUTS_SWITCH_OFF,
	OUTPUTS_SWITCH_ON
};

#define OUTPUTS_LED1_GPIO     (GPIO_PORTA)
#define OUTPUTS_LED2_GPIO     (GPIO_PORTA)
#define OUTPUTS_SERVO_GPIO    (GPIO_PORTA)
#define OUTPUTS_SBUSPOL_GPIO  (GPIO_PORTA)
#define OUTPUTS_SBUSTE_GPIO   (GPIO_PORTA)

#define OUTPUTS_LED1_PIN      (2)
#define OUTPUTS_LED2_PIN      (3)
#define OUTPUTS_SERVO_PIN     (16)
#define OUTPUTS_SBUSPOL_PIN   (27)
#define OUTPUTS_SBUSTE_PIN    (18)

void OUTPUTS_Configuration(void);
void OUTPUTS_Switch(uint8_t num, uint8_t on);
void OUTPUTS_Toggle(uint8_t num);
bool OUTPUTS_IsActive(uint8_t num);

#endif
