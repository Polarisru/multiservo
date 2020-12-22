#include "driver_clk.h"
#include "driver_gpio.h"
#include "keeloq.h"
#include "rs485.h"
#include "canbus.h"
#include "global.h"

#include "driver_dma.h"
#include "driver_temp.h"
#include "driver_timer.h"
#include "driver_uart.h"

/**< Main RTOS task with periodical actions */
void MainTask(void *pParameters)
{
  #define BIT8    (1UL << 8)
  (void) pParameters;   /* to quiet warnings */
  uint32_t ticks = 0;

	while (1)
  {
    vTaskDelay(1);

    if (xTaskGetTickCount() >= ticks)
    {
      /**< Every 1 second */
      ticks += 1000;

      /**< Increment working time counter */
      KEELOQ_Write(0x0A, 0xFF);
    }
	}
}

/**< Initializing task, will be suspended after completion */
void InitTask(void *pParameters)
{
  BaseType_t xRet;

  GPIO_Init();

  KEELOQ_Init();

  /**< Initialize and check EEPROM */
  //EEPROM_Init();

  /**< Create main task for periodical actions with low priority */
  xRet = xTaskCreate(MainTask, (const char *) "Main Task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
  /**< Create ADC task with lowest priority */
  //xRet = xTaskCreate(MONITOR_Task, (const char *) "ADC Task", MONITOR_TASK_STACK_SIZE, NULL, MONITOR_TASK_PRIORITY, NULL);

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
