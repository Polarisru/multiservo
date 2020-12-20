#include "driver_clk.h"
#include "driver_gpio.h"
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
  //uint8_t buff[2] = {0x0A, 0xD0};
  //char str[] = "1234\n";
  //char str2[8];
  //uint32_t buff[16] = {BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8, BIT8};
  //TDmaSettings DmaSett;
  //float temp;

  // *****
//  GPIO_ClearPin(GPIO_PORTA, 8);
//  GPIO_SetDir(GPIO_PORTA, 8, true);
//  GPIO_SetFunction(GPIO_PORTA, 8, GPIO_PIN_FUNC_OFF);
//
//	GPIO_SetFunction(GPIO_PORTA, 18, MUX_PA18C_SERCOM1_PAD2);  // TX
//	GPIO_SetFunction(GPIO_PORTA, 19, MUX_PA19C_SERCOM1_PAD3);  // RX
//  UART_Init(SERCOM1, 3/*PAD[3]*/, 1 /*PAD[2]*/, 115200);
//  UART_SendByte(SERCOM1, 0x41);
//  DMA_Init();
//  TIMER_Init();
//  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_BYTE_Val;//DMAC_BTCTRL_BEATSIZE_WORD_Val;
//  DmaSett.trig_src = SERCOM1_DMAC_ID_TX;//TC1_DMAC_ID_OVF;
//  DmaSett.dst_addr = (void*)&SERCOM1->USART.DATA.reg;//PORT->Group[GPIO_PORTA].OUTTGL.reg;
//  DmaSett.src_addr = (void*)str;
//  DmaSett.src_inc = true;
//  DmaSett.dst_inc = false;
//  DmaSett.len = strlen(str);//sizeof(buff) / sizeof(uint32_t);
//  DmaSett.priority = 0;
//  //TIMER_GenerateUs(300);
//  DMA_StartChannel(0, &DmaSett);
//  TEMP_Init();
  // *****

	while (1)
  {
    vTaskDelay(1);

    CAN0->ECR.reg;

    if (xTaskGetTickCount() >= ticks)
    {
      /**< Every 1 second */
      ticks += 1000;

      /**< Increment working time counter */
      //COUNTER_Inc();  - disabled because of SPI collisions!
      //KEELOQ_Send(buff);
//      temp = TEMP_GetValue();
//      sprintf(str, "%2d.%1d\n", (uint16_t)temp, (uint16_t)(temp * 10) % 10);
//      DMA_RestartChannel(0);
      //memset(str2, 0, sizeof(str2));
    }
	}
}

/**< Initializing task, will be suspended after completion */
void InitTask(void *pParameters)
{
  BaseType_t xRet;

  GPIO_Init();

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
