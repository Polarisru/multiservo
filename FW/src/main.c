#include "driver_clk.h"
#include "driver_gpio.h"
#include "driver_temp.h"
#include "analog.h"
#include "canbus.h"
#include "comm.h"
#include "eeprom.h"
#include "global.h"
#include "keeloq.h"
#include "outputs.h"
#include "pwmout.h"
#include "rs485.h"

#include "driver_dma.h"

/**< Main RTOS task with periodical actions */
void MainTask(void *pParameters)
{
  (void) pParameters;   /* to quiet warnings */
  uint32_t ticks = 0;
  //uint8_t counter = 0;
  //uint8_t data[4] = {0, 1, 2, 3};
  //uint32_t id = 0x123;

	while (1)
  {
    vTaskDelay(1);

    if (xTaskGetTickCount() >= ticks)
    {
      /**< Every 1 second */
      ticks += 1000;
      /**< Increment working time counter */
      //KEELOQ_Write(0x0A, 0xFF);
      OUTPUTS_Toggle(OUTPUTS_LED2);

//      counter++;
//      switch (counter % 4)
//      {
//        case 0:
//          CANBUS_SendMessage(id, data, sizeof(data), false, false, false);
//          break;
//        case 1:
//          CANBUS_SendMessage(id, data, sizeof(data), true, false, false);
//          break;
//        case 2:
//          CANBUS_SendMessage(id, data, sizeof(data), true, true, false);
//          break;
//        case 3:
//          CANBUS_SendMessage(id, data, sizeof(data), true, true, true);
//          break;
//      }
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
  //CANBUS_SetBaudrate(1000, 2000);

  OUTPUTS_Switch(OUTPUTS_LED1, OUTPUTS_SWITCH_ON);

  //KEELOQ_Init();

  /**< Initialize and check EEPROM */
  EEPROM_Init();

  /**< Create main task for periodical actions with low priority */
  xRet = xTaskCreate(MainTask, (const char *) "Main Task", MAIN_TASK_STACK_SIZE, NULL, MAIN_TASK_PRIORITY, NULL);
  /**< Create communication task with high priority */
  xRet = xTaskCreate(COMM_Task, (const char *) "Comm Task", COMM_TASK_STACK_SIZE, NULL, COMM_TASK_PRIORITY, &xTaskComm);
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
