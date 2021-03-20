#include "drivers.h"
#include "analog.h"
#include "canbus.h"
#include "comm.h"
#include "eeprom.h"
#include "global.h"
#include "keeloq.h"
#include "outputs.h"
#include "pwmout.h"
#include "rs485.h"
#include "sbus.h"
#include "serial.h"

/**< Main RTOS task with periodical actions */
void MainTask(void *pParameters)
{
  (void) pParameters;   /* to quiet warnings */
  uint32_t ticks = 0;

  GLOBAL_PowerOn = false;

	while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(1));

    if (xTaskGetTickCount() >= ticks)
    {
      /**< Every 500 milliseconds, increment working time counter */
      ticks += pdMS_TO_TICKS(500);
      /**< Toggle power LED if power is not applied */
      if (GLOBAL_PowerOn == false)
        OUTPUTS_Toggle(OUTPUTS_LED1);
      else
        OUTPUTS_Switch(OUTPUTS_LED1, OUTPUTS_SWITCH_ON);
    }
	}
}

/**< Initializing task, will be suspended after completion */
void InitTask(void *pParameters)
{
  (void) pParameters;   /* to quiet warnings */
  BaseType_t xRet;

  /**< Create event group */
  xEventGroupCommon = xEventGroupCreate();

  GPIO_Init();
  TEMP_Init();
  DMA_Init();

  OUTPUTS_Configuration();
  PWMOUT_Configuration();
  PWMOUT_Enable();
  PWMOUT_SetValue(0.0f);
  KEELOQ_Configuration();
  RS485_Configuration();
  ANALOG_Configuration();
  CANBUS_Configuration();
  SERIAL_Configuration();
  SBUS_Configuration();

  OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_OFF);

  /**< Initialize and check EEPROM */
  EEPROM_Init();

  /**< Create main task for periodical actions with low priority */
  xRet = xTaskCreate(MainTask, (const char *) "Main Task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
  /**< Create communication task with high priority */
  xRet = xTaskCreate(COMM_Task, (const char *) "Comm Task", COMM_TASK_STACK_SIZE, NULL, COMM_TASK_PRIORITY, &xTaskComm);
  /**< Create SBUS task with lowest priority and suspend it */
  xRet = xTaskCreate(SBUS_Task, (const char *) "SBUS Task", SBUS_TASK_STACK_SIZE, NULL, SBUS_TASK_PRIORITY, &xTaskSbus);
  vTaskSuspend(xTaskSbus);

  /**< Suspend current initialization task, is not deleted because of heap_1, dynamically memory configuration is not wished */
  //vTaskPrioritySet(NULL, tskIDLE_PRIORITY);
  vTaskSuspend(NULL);
}

int main(void)
{
	CLK_Init();

  /**< Create initializing task because of usage of RTOS functions during EEPROM initialization */
  xTaskCreate(InitTask, (const char *) "Init Task", INIT_TASK_STACK_SIZE, NULL, INIT_TASK_PRIORITY, NULL);

  /**< Start FreeRTOS Scheduler */
  vTaskStartScheduler();
}
