#include <string.h>
#include "err_codes.h"
#include "driver_can.h"
#include "driver_wdt.h"
#include "canbus.h"
#include "arinc825.h"
#include "defines.h"
#include "eeprom.h"
#include "rs485.h"
#include "meteor.h"
#include "monitor.h"
#include "motor.h"
#include "global.h"

/* The queue is to be created to hold a maximum of 10 uint64_t variables. */
#define CAN_QUEUE_LENGTH    10
#define CAN_ITEM_SIZE       sizeof(TCanMsg)

QueueHandle_t xQueueCAN;

/*
 * \brief CAN interrupt handler
 */
void CAN0_Handler(void)
{
  uint32_t ir;
  BaseType_t xHigherPriorityTaskWoken;

  ir = CAN0->IR.reg;

  /**< We have not woken a task at the start of the ISR */
  xHigherPriorityTaskWoken = pdFALSE;

  if (ir & CAN_IR_RF0N)
  {
    struct can_message msg;
    uint8_t data[64];
    TCanMsg CanMsg;

    msg.data = data;
    CAN_ReadMsg(&msg);
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

/** \brief RTOS task for processing CAN connection
 */
void CANBUS_Task(void *pParameters)
{
  TCanMsg rx_msg;
  uint8_t tx_data[CANBUS_BUF_LEN];
  struct can_message msg;
  struct can_filter filter;
  uint16_t uval16;
  uint8_t uval8;
  uint32_t uval32;
  int16_t tmp = 0;
  uint32_t id;
  uint8_t rec_id;
  uint8_t cmd, resp_cmd;
  eeVal_t param;

  /**< Initialize CAN connection */
  CAN_Init();
  RS485_Disable();

  #ifdef TEST_BOARD
  EE_Id = 2;
  EE_CanId = 0x3f0;
  #endif

  EE_CanId &= CANBUS_ID_OWN_MASK;
  EE_Id &= CANMSG_ID_MASK;

  /**< Add main CAN filter */
	filter.id   = CANBUS_BACKDOOR_ID;
	filter.mask = CANBUS_ID_MASK;
	CAN_SetFilter(0, CAN_FMT_STDID, &filter);
	/**< Add CAN filter for the bootloader */
	filter.id   = EE_CanId & CANBUS_ID_OWN_MASK;
	filter.mask = CANBUS_ID_OWN_MASK;
	CAN_SetFilter(1, CAN_FMT_STDID, &filter);
	//CAN_SetRangeFilter(0, CAN_FMT_EXTID, CANBUS_NOC_FILTER, CANBUS_NOC_FILTER + CANBUS_SERVO_MAX_ID);
	//CAN_SetRangeFilter(1, CAN_FMT_EXTID, CANBUS_TMC_FILTER, CANBUS_TMC_FILTER + CANBUS_SERVO_MAX_ID);
	/**< Enable CAN bus */
  CAN_Enable();

  /**< Create a queue capable of containing CAN packets */
  xQueueCAN = xQueueCreate(CAN_QUEUE_LENGTH, CAN_ITEM_SIZE);

  /**< Prepare message structure for replies */
  msg.type = CAN_TYPE_DATA;
  msg.data = tx_data;

  while (1)
  {
    xQueueReceive(xQueueCAN, &rx_msg, portMAX_DELAY);
    if (rx_msg.fmt == CAN_FMT_STDID)
    {
      if (rx_msg.data[CANMSG_OFFSET_CMD] & CANMSG_CMD_REPLY_BIT)
        continue;
      rec_id = (uint8_t)(rx_msg.id & CANMSG_ID_MASK);
      if (((rx_msg.id & CANBUS_ID_OWN_MASK) == EE_CanId)&& ((rec_id == EE_Id) || (rec_id == CANMSG_ID_BROADCAST)))
      {
        msg.fmt  = CAN_FMT_STDID;
        cmd = rx_msg.data[CANMSG_OFFSET_CMD] & CANMSG_CMD_MASK;
        msg.id = EE_CanId | EE_Id;
        tx_data[CANMSG_OFFSET_CMD] = cmd | CANMSG_CMD_REPLY_BIT;
        msg.len = CANMSG_OFFSET_DATA;
        /**< Standard CAN message */
        switch (cmd)
        {
          case CANMSG_CMD_ECHO:
            /**< Just echo current message */
            msg.len = rx_msg.len;
            break;
          case CANMSG_CMD_SET_POS:
            /**< Set motor position */
            tmp = (rx_msg.data[CANMSG_OFFSET_DATA] << 8) + rx_msg.data[CANMSG_OFFSET_DATA + 1];     // desired position complete
            if (tmp < MOTOR_MIN_TP)
              tmp = MOTOR_MIN_TP;
            if (tmp > MOTOR_MAX_TP)
              tmp = MOTOR_MAX_TP;
            GLOBAL_TargetPos = tmp;
            tx_data[CANMSG_OFFSET_DATA] = GLOBAL_RealPos >> 8;
            tx_data[CANMSG_OFFSET_DATA + 1] = (uint8_t)GLOBAL_RealPos;
            msg.len += 2;
            break;
//          case CANMSG_CMD_SET_POS_GROUP:
//            /**< Set position for group of servos */
//            tx_data[CANMSG_OFFSET_DATA] = GLOBAL_RealPos >> 8;
//            tx_data[CANMSG_OFFSET_DATA + 1] = (uint8_t)GLOBAL_RealPos;
//            msg.len += 2;
//            break;
          case CANMSG_CMD_GET_POS:
            /**< Read magnet sensor, status, current, voltage */
            uval16 = GLOBAL_RealPos;
            tx_data[CANMSG_OFFSET_DATA] = (uint8_t)(uval16 >> 8);
            tx_data[CANMSG_OFFSET_DATA + 1] = (uint8_t)uval16;
            tx_data[CANMSG_OFFSET_DATA + 2] = 0;    // reserved for status
            tx_data[CANMSG_OFFSET_DATA + 3] = MONITOR_GetCurrent();
            tx_data[CANMSG_OFFSET_DATA + 4] = MONITOR_GetVoltage();
            msg.len += 5;
            break;
          case CANMSG_CMD_GET_STAT:
            /**< Get extended status */
            break;
          case CANMSG_CMD_GET_TIME:
            uval32 = xTaskGetTickCount() / configTICK_RATE_HZ;
            memcpy(&tx_data[CANMSG_OFFSET_DATA], &uval32, sizeof(uint32_t));
            msg.len += sizeof(uint32_t);
            break;
          case CANMSG_CMD_GET_PARAM:
            /**< Read servo parameter */
            uval8 = rx_msg.data[CANMSG_OFFSET_DATA] & CANMSG_PARAM_MASK;
            if (EEPROM_GetParam(uval8, &param) == false)
            {
              msg.len = 0;
              break;
            }
            tx_data[CANMSG_OFFSET_DATA] = rx_msg.data[CANMSG_OFFSET_DATA];
            /**< Type of parameter */
            tx_data[CANMSG_OFFSET_DATA + 1] = param.type;
            msg.len += 2;
            switch (rx_msg.data[CANMSG_OFFSET_DATA] >> CANMSG_PARAM_WIDTH)
            {
              case CANMSG_PARAM_VAL:
                memcpy(&tx_data[CANMSG_OFFSET_DATA + 2], param.pVal, param.size);
                msg.len += param.size;
                break;
              case CANMSG_PARAM_MINVAL:
                memcpy(&tx_data[CANMSG_OFFSET_DATA + 2], &param.minVal, param.size);
                msg.len += param.size;
                break;
              case CANMSG_PARAM_MAXVAL:
                memcpy(&tx_data[CANMSG_OFFSET_DATA + 2], &param.maxVal, param.size);
                msg.len += param.size;
                break;
              case CANMSG_PARAM_NAME:
                memset(&tx_data[CANMSG_OFFSET_DATA + 1], 0, PARAM_NAME_LEN);
                strcpy((char*)&tx_data[CANMSG_OFFSET_DATA + 1], param.name);
                msg.len--;
                msg.len += PARAM_NAME_LEN;
                break;
            }
            break;
          case CANMSG_CMD_SET_PARAM:
            /**< Write servo parameter */
            uval8 = rx_msg.data[CANMSG_OFFSET_DATA] & CANMSG_PARAM_MASK;
            if (EEPROM_GetParam(uval8, &param) == false)
            {
              msg.len = 0;
              break;
            }
            if ((rx_msg.data[CANMSG_OFFSET_DATA] >> CANMSG_PARAM_WIDTH) != CANMSG_PARAM_VAL)
            {
              msg.len = 0;
              break;
            }
            EEPROM_SetParam(uval8, (void*)&rx_msg.data[CANMSG_OFFSET_DATA + 1]);
            //memcpy(param.pVal, &rx_msg.data[CANMSG_OFFSET_DATA + 1], param.size);
            break;
          case CANMSG_CMD_GET_NAME:
            /**< Get servo title */
            memcpy(&tx_data[CANMSG_OFFSET_DATA], EE_Name, SERVO_NAME_LEN);
            msg.len += SERVO_NAME_LEN;
            break;
          case CANMSG_CMD_SET_NAME:
            /**< Get servo title */
            memcpy(EE_Name, &tx_data[CANMSG_OFFSET_DATA], SERVO_NAME_LEN);
            memcpy(&rx_msg.data[CANMSG_OFFSET_DATA], &tx_data[CANMSG_OFFSET_DATA], SERVO_NAME_LEN);
            msg.len += SERVO_NAME_LEN;
            break;
          case CANMSG_CMD_GET_SN:
            /**< Get serial number */
            //memcpy(&tx_data[CANMSG_OFFSET_DATA], , );
            //msg.len += 1;
            break;
          case CANMSG_CMD_READ_EE:
            /**< Read EEPROM data */
            tx_data[CANMSG_OFFSET_DATA] = rx_msg.data[CANMSG_OFFSET_DATA];
            tx_data[CANMSG_OFFSET_DATA + 1] = EEPROM_ReadByte((uint16_t)rx_msg.data[CANMSG_OFFSET_DATA]);
            msg.len += 2;
            break;
          case CANMSG_CMD_WRITE_EE:
            /**< Write EEPROM data */
            //EEPROM_WriteByteSecure((uint16_t)rx_msg.data[CANMSG_OFFSET_DATA], rx_msg.data[CANMSG_OFFSET_DATA + 1]);
            EEPROM_WriteByte((uint16_t)rx_msg.data[CANMSG_OFFSET_DATA], rx_msg.data[CANMSG_OFFSET_DATA + 1]);
            tx_data[CANMSG_OFFSET_DATA] = rx_msg.data[CANMSG_OFFSET_DATA];
            tx_data[CANMSG_OFFSET_DATA + 1] = rx_msg.data[CANMSG_OFFSET_DATA + 1];
            msg.len += 2;
            break;
          case CANMSG_CMD_BOOTLOADER:
            /**< Start bootloader */
            if (rx_msg.data[CANMSG_OFFSET_DATA] == FLASH_CMD_STARTBL)
            {
              WDT_Init();
              WDT_Enable();
            }
            break;
          default:
            msg.len = 0;
            break;
        }
      } else
      if (rx_msg.id == CANBUS_BACKDOOR_ID)
      {
        /**< Backdoor ID to get real IDs */
        msg.id = EE_CanId | EE_Id;
        tx_data[CANMSG_OFFSET_CMD] = CANMSG_CMD_ECHO | CANMSG_CMD_REPLY_BIT;
        msg.len = CANMSG_OFFSET_DATA;
      } else
      {
        /**< Unknown command or ID, don't reply */
        continue;
      }
      if (msg.len > 0)
        CAN_WriteMsg(&msg);
    } else
    {
      /**< Extended CAN message */
      msg.fmt  = CAN_FMT_EXTID;
      id = rx_msg.id;
      rec_id = (id & CANBUS_MASK_ID) >> CANBUS_OFFS_ID;
      if ((rec_id != CANBUS_BROADCAST) && (rec_id != EE_Id))
        /**< False ID, don't process this packet */
        continue;
      cmd = (id & CANBUS_MASK_CMD) >> CANBUS_OFFS_CMD;
      switch (cmd)
      {
        case CAN_CMD_SETPOS:
          /**< Set motor position */
          tmp = (rx_msg.data[0] << 8) + rx_msg.data[1];     // desired position complete
          if (tmp < MOTOR_MIN_TP)
            tmp = MOTOR_MIN_TP;
          if (tmp > MOTOR_MAX_TP)
            tmp = MOTOR_MAX_TP;
          GLOBAL_TargetPos = tmp;
          uval16 = GLOBAL_RealPos;
          tx_data[0] = (uint8_t)(uval16 >> 8);
          tx_data[1] = (uint8_t)uval16;
          tx_data[2] = MONITOR_GetCurrent();
          tx_data[3] = MONITOR_GetVoltage();
          tx_data[4] = tx_data[3];
          tx_data[5] = 0;
          tx_data[6] = 0;
          msg.len  = 7;
          resp_cmd = CAN_CMD_SETPOS + 1;
          break;
        case CAN_CMD_GETPOS:
          /**< Get motor position */
          uval16 = GLOBAL_RealPos;
          tx_data[0] = (uint8_t)(uval16 >> 8);
          tx_data[1] = (uint8_t)uval16;
          tx_data[2] = MONITOR_GetCurrent();
          tx_data[3] = MONITOR_GetVoltage();
          tx_data[4] = tx_data[3];
          tx_data[5] = 0;
          tx_data[6] = 0;
          msg.len  = 7;
          resp_cmd = CAN_CMD_GETPOS;
          break;
        case CAN_CMD_GETCOUNTERS:
          /**< Get load counters */
          msg.len = 8;
          memset(tx_data, 0, 8);
          resp_cmd = CAN_CMD_GETCOUNTERS + 1;
          break;
        case CAN_CMD_GETSTATUS:
          /**< Get extended status */
          msg.len  = 8;
          memset(tx_data, 0, 8);
          resp_cmd = CAN_CMD_GETSTATUS + 1;
          break;
        case CAN_CMD_SETID:
          /**< Change ID of the servo */
          tx_data[0] = rx_msg.data[0];
          if (rx_msg.data[0] == 0)
          {
            /**< Set ID */
            uval8 = rx_msg.data[1];
            if ((uval8 <= MAX_ID_VALUE) && (uval8 != 0))
            {
              EE_Id = uval8;
              //EEPROM_WriteByteSecure(0x01, EE_Id);
              EEPROM_SaveVariable(&EE_Id);
            }
          }
          msg.len  = 2;
          tx_data[1] = EE_Id;
          resp_cmd = CAN_CMD_SETID + 1;
          break;
        case CAN_CMD_GETVER:
          /**< Get HW and FW versions */
          msg.len  = 8;
          memset(tx_data, 0, 8);
          resp_cmd = CAN_CMD_GETVER + 1;
          break;
        case CAN_CMD_GETWORKTIME:
          /**< Get working time */
          tx_data[0] = GLOBAL_WorkingTime % 60;
          tx_data[1] = (GLOBAL_WorkingTime % 3600) / 60;
          tx_data[2] = (uint8_t)((GLOBAL_WorkingTime / 3600) >> 8);
          tx_data[3] = (uint8_t)(GLOBAL_WorkingTime / 3600);
          msg.len  = 4;
          resp_cmd = CAN_CMD_GETWORKTIME + 1;
          break;
        case CAN_CMD_WRITEACCESS:
          /**< Write access for the next EEPROM operation */
          msg.len  = 0;
          resp_cmd = CAN_CMD_WRITEACCESS + 1;
          break;
        case CAN_CMD_READEEPROM:
          /**< Read something from EEPROM (1 byte) */
          tx_data[0] = rx_msg.data[0];
          tx_data[1] = EEPROM_ReadByte((uint16_t)rx_msg.data[0]);
          tx_data[2] = 0;
          tx_data[3] = 0;
          msg.len  = 4;
          resp_cmd = CAN_CMD_READEEPROM + 1;
          break;
        case CAN_CMD_WRITEEEPROM:
          /**< Write something to EEPROM (1 byte) */
          tx_data[0] = rx_msg.data[0];
          tx_data[1] = rx_msg.data[1];
          //EEPROM_WriteByteSecure((uint16_t)tx_data[0], tx_data[1]);
          EEPROM_WriteByte((uint16_t)tx_data[0], tx_data[1]);
          tx_data[2] = 0;
          tx_data[3] = 0;
          msg.len  = 4;
          resp_cmd = CAN_CMD_WRITEEEPROM + 1;
          break;

        default:
          resp_cmd = 0;
          break;
      }
      if (resp_cmd != 0)
      {
        id = METEOR_BuildReplyID(id, resp_cmd);
        msg.id = id;
        CAN_WriteMsg(&msg);
      }
    }
  }
}

bool CANBUS_Send(uint32_t Id, uint8_t *data, uint8_t datalen)
{
  struct can_message msg;

  msg.id = Id;
  msg.type = CAN_TYPE_DATA;
  msg.data = data;
  msg.len = datalen;
  msg.fmt  = CAN_FMT_EXTID;
  return (CAN_WriteMsg(&msg) == ERR_NONE);
}
