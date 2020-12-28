#include <string.h>
#include "err_codes.h"
#include "driver_can.h"
#include "driver_gpio.h"
#include "driver_wdt.h"
#include "canbus.h"
#include "defines.h"
#include "global.h"

#define CAN_PORT            GPIO_PORTA
#define CAN_PIN_RX          25
#define CAN_PIN_TX          24

#define CAN_MUX_RX          MUX_PA25G_CAN0_RX
#define CAN_MUX_TX          MUX_PA24G_CAN0_TX

#define CAN_CHANNEL         CAN0
#define CAN_INTHANDLER      CAN0_Handler
#define CAN_IRQ             CAN0_IRQn

/**< The queue is to be created to hold a maximum of 10 uint64_t variables */
#define CAN_QUEUE_LENGTH    10
#define CAN_ITEM_SIZE       sizeof(TCanMsg)

QueueHandle_t xQueueCAN;

/**< CAN interrupt handler */
void CAN_INTHANDLER(void)
{
  uint32_t ir;
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

  ir = CAN_CHANNEL->IR.reg;

  if (ir & CAN_IR_RF0N)
  {
    struct can_message msg;
    uint8_t data[64];
    TCanMsg CanMsg;

    msg.data = data;
    CAN_ReadMsg(CAN_CHANNEL, &msg);
    CanMsg.id = msg.id;
    CanMsg.fmt = msg.fmt;
    CanMsg.len = msg.len;
    memcpy(CanMsg.data, msg.data, msg.len);
    xQueueSendFromISR(xQueueCAN, &CanMsg, &xHigherPriorityTaskWoken);
  }

//	if (ir & CAN_IR_TC) {
//		//dev->cb.tx_done(dev);
//	}
//
//	if (ir & CAN_IR_BO) {
//		//dev->cb.irq_handler(dev, CAN_IRQ_BO);
//	}
//
//	if (ir & CAN_IR_EW) {
//		//dev->cb.irq_handler(dev, CAN_IRQ_EW);
//	}
//
//	if (ir & CAN_IR_EP) {
//		//dev->cb.irq_handler(dev, hri_can_get_PSR_EP_bit(dev->hw) ? CAN_IRQ_EP : CAN_IRQ_EA);
//	}
//
//	if (ir & CAN_IR_RF0L) {
//		//dev->cb.irq_handler(dev, CAN_IRQ_DO);
//	}

	CAN_CHANNEL->IR.reg = ir;

  /**< Now the buffer is empty we can switch context if necessary */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

bool CANBUS_Send(uint32_t Id, uint8_t *data, uint8_t datalen)
{
  struct can_message msg;

  msg.id = Id;
  msg.type = CAN_TYPE_DATA;
  msg.data = data;
  msg.len = datalen;
  msg.fmt  = CAN_FMT_EXTID;
  return (CAN_WriteMsg(CAN_CHANNEL, &msg) == ERR_NONE);
}

bool CANBUS_SetBaudrate(uint32_t nominal_baudrate, uint32_t data_baudrate)
{
  CAN_Disable(CAN_CHANNEL);
  CAN_SetBaudrate(CAN_CHANNEL, nominal_baudrate, data_baudrate);
  CAN_Enable(CAN_CHANNEL);
  return true;
}

/** \brief Deinitialize CAN module
 *
 * \return Nothing
 *
 */
void CANBUS_Disable(void)
{
  CAN_Disable(CAN_CHANNEL);
}

/** \brief Initialize CAN communication module
 *
 * \return Nothing
 *
 */
void CANBUS_Configuration(void)
{
  CAN_Init(CAN_CHANNEL);

  /**< Enable CAN interrupt */
  NVIC_DisableIRQ(CAN_IRQ);
  NVIC_ClearPendingIRQ(CAN_IRQ);
  NVIC_EnableIRQ(CAN_IRQ);
  CAN_CHANNEL->ILE.reg = CAN_ILE_EINT0;

  /**< Configure CAN pins */
  /**< There is only one available pin configuration for this MCU */
  GPIO_SetFunction(CAN_PORT, CAN_PIN_TX, CAN_MUX_TX);
  GPIO_SetFunction(CAN_PORT, CAN_PIN_RX, CAN_MUX_RX);

  /**< Setup CAN filters */
	//CAN_SetRangeFilter(0, CAN_FMT_EXTID, CANBUS_NOC_FILTER, CANBUS_NOC_FILTER + CANBUS_SERVO_MAX_ID);
	//CAN_SetRangeFilter(1, CAN_FMT_EXTID, CANBUS_TMC_FILTER, CANBUS_TMC_FILTER + CANBUS_SERVO_MAX_ID);

	CAN_Enable(CAN_CHANNEL);
}
