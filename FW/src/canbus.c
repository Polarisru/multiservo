#include <string.h>
#include "err_codes.h"
#include "driver_can.h"
#include "driver_wdt.h"
#include "canbus.h"
#include "defines.h"
#include "global.h"

/* The queue is to be created to hold a maximum of 10 uint64_t variables. */
#define CAN_QUEUE_LENGTH    10
#define CAN_ITEM_SIZE       sizeof(TCanMsg)

#define CAN_CHANNEL         CAN0

QueueHandle_t xQueueCAN;

/*
 * \brief CAN interrupt handler
 */
void CAN0_Handler(void)
{
  uint32_t ir;
  BaseType_t xHigherPriorityTaskWoken;

  ir = CAN_CHANNEL->IR.reg;

  /**< We have not woken a task at the start of the ISR */
  xHigherPriorityTaskWoken = pdFALSE;

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

	CAN0->IR.reg = ir;

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

void CANBUS_SetBaudrate(uint32_t nominal_baudrate, uint32_t data_baudrate)
{

}
