#ifndef OUTPUTS_H
#define OUTPUTS_H

#include "defines.h"

enum
{
  OUTPUTS_LED1,
  OUTPUTS_LED2,
  OUTPUTS_SERVO,
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

#define OUTPUTS_LED1_PIN 		  (4)
#define OUTPUTS_LED2_PIN 		  (5)
#define OUTPUTS_SERVO_PIN 		(10)

void OUTPUTS_Configuration(void);
void OUTPUTS_Switch(uint8_t num, uint8_t on);
bool OUTPUTS_IsActive(uint8_t num);

#endif
