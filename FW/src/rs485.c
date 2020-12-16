#include "driver_gpio.h"
#include "driver_uart.h"
#include "counter.h"
#include "crc16.h"
#include "cmd_set.h"
#include "defines.h"
#include "eeprom.h"
#include "global.h"
#include "magnet.h"
#include "monitor.h"
#include "motor.h"
#include "rs485.h"

#define RS485_PORT          GPIO_PORTA
#define RS485_PIN_DIR       23
#define RS485_PIN_TE        27
#define RS485_PIN_TX        24
#define RS485_PIN_RX        25

#define RS485_RXPO          3
#define RS485_TXPO          1

#define RS485_CHANNEL       SERCOM3
#define RS485_BAUDRATE      115200

#define RS485_PACKET_LEN    6
#define RS485_CRC_LEN       sizeof(uint16_t)
#define RS485_CMD_OFFSET    0
#define RS485_ID_OFFSET     1
#define RS485_DATA_OFFSET   2
#define RS485_CRC_OFFSET    (RS485_PACKET_LEN - RS485_CRC_LEN)

/**< The queue is to be created to hold a maximum of 10 uint64_t variables. */
#define RS485_QUEUE_LENGTH  10
#define RS485_ITEM_SIZE     sizeof(uint8_t) * RS485_CRC_OFFSET

#define fixed_ID            0x01
#define RS485_BROADCAST_ID  0x1F

QueueHandle_t xQueueRS485;
uint8_t Station_ID;

/** \brief SERCOM3 interrupt handler to process RS485 messages
 *
 * \return Nothing
 *
 */
void SERCOM3_Handler(void)
{
  uint8_t ch, i;
  uint16_t crc;
  static uint8_t buffer[RS485_PACKET_LEN] = {0};
  BaseType_t xHigherPriorityTaskWoken;

  /**< We have not woken a task at the start of the ISR */
  xHigherPriorityTaskWoken = pdFALSE;

  if ((RS485_CHANNEL->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) >> SERCOM_USART_INTFLAG_RXC_Pos)
  {
    if (RS485_CHANNEL->USART.STATUS.reg & (SERCOM_USART_STATUS_PERR | SERCOM_USART_STATUS_FERR |
                                     SERCOM_USART_STATUS_BUFOVF | SERCOM_USART_STATUS_ISF | SERCOM_USART_STATUS_COLL))
    {
			RS485_CHANNEL->USART.STATUS.reg = SERCOM_USART_STATUS_MASK;
			return;
		}
		ch = RS485_CHANNEL->USART.DATA.reg;
		/**< shift receiving buffer */
		for (i = 0; i < RS485_PACKET_LEN - 1; i++)
      buffer[i] = buffer[i + 1];
    buffer[RS485_PACKET_LEN - 1] = ch;
    if ((buffer[RS485_ID_OFFSET] == Station_ID) || (buffer[RS485_ID_OFFSET] == RS485_BROADCAST_ID))
    {
      /**< have got our ID or broadcast ID */
      crc = CRC16_CalcKearfott(buffer, RS485_PACKET_LEN - RS485_CRC_LEN);
      if ((uint8_t)(crc >> 8) == buffer[RS485_CRC_OFFSET] && (uint8_t)crc == buffer[RS485_CRC_OFFSET + 1])
        /**< CRC is Ok, place packet to RS485 processing queue */
        xQueueSendFromISR(xQueueRS485, buffer, &xHigherPriorityTaskWoken);
    }
  }

  /**< Now the buffer is empty we can switch context if necessary */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/** \brief Initialize RS485 module
 *
 * \return Nothing
 *
 */
void RS485_Init(void)
{
  /**< Configure UART */
  UART_Init(RS485_CHANNEL, RS485_RXPO, RS485_TXPO, RS485_BAUDRATE);

  /**< configure DIR pin and TE pin */
  GPIO_ClearPin(RS485_PORT, RS485_PIN_DIR);
  GPIO_ClearPin(RS485_PORT, RS485_PIN_TE);
  GPIO_SetDir(RS485_PORT, RS485_PIN_DIR, true);
  GPIO_SetDir(RS485_PORT, RS485_PIN_TE, true);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_DIR, GPIO_PIN_FUNC_OFF);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_TE, GPIO_PIN_FUNC_OFF);
  /**< Setup TX/RX pins */
	GPIO_SetFunction(RS485_PORT, RS485_PIN_TX, MUX_PA24C_SERCOM3_PAD2);
	GPIO_SetFunction(RS485_PORT, RS485_PIN_RX, MUX_PA25C_SERCOM3_PAD3);

  /**< enable receiving interrupt */
  RS485_CHANNEL->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
  NVIC_EnableIRQ(SERCOM3_IRQn);
}

/** \brief Switch RS485 internal termination resistor
 *
 * \param [in] on True if enable, otherwise disable
 * \return Nothing
 *
 */
void RS485_SwitchTermination(bool on)
{
  if (on)
    GPIO_SetPin(RS485_PORT, RS485_PIN_TE);
  else
    GPIO_ClearPin(RS485_PORT, RS485_PIN_TE);
}

/** \brief Send data packet via RS485 bus
 *
 * \param [in] data Buffer with data to send
 * \param [in] len Length of data packet
 * \return Nothing
 *
 */
void RS485_SendPacket(uint8_t *data, uint8_t len)
{
  RS485_CHANNEL->USART.CTRLB.bit.RXEN = 0;
  GPIO_SetPin(RS485_PORT, RS485_PIN_DIR);

  while (len--)
    UART_SendByte(RS485_CHANNEL, *data++);

  GPIO_ClearPin(RS485_PORT, RS485_PIN_DIR);
  RS485_CHANNEL->USART.CTRLB.bit.RXEN = 1;
}

/** \brief Disable RS485
 *
 * \return Nothing
 *
 */
void RS485_Disable(void)
{
  GPIO_SetDir(RS485_PORT, RS485_PIN_DIR, true);
  GPIO_SetPin(RS485_PORT, RS485_PIN_DIR);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_DIR, GPIO_PIN_FUNC_OFF);
}

/** \brief RTOS task for processing RS485 connection
 */
void RS485_Task(void *pParameters)
{
  uint8_t cmd = 0, arg_1 = 0, arg_2 = 0;
  uint8_t i;
  int16_t tmp = 0;
  uint16_t crc;
  int32_t ltmp;
  uint8_t tx_buffer[RS485_PACKET_LEN];
  uint8_t rx_buffer[RS485_CRC_OFFSET];
  bool doAnswer;
  static bool access_eeprom_bit = false;

  /**< Create a queue capable of containing RS485 packets */
  xQueueRS485 = xQueueCreate(RS485_QUEUE_LENGTH, RS485_ITEM_SIZE);

  RS485_Init();

  Station_ID = fixed_ID;
  tx_buffer[RS485_ID_OFFSET] = Station_ID;

  while (1)
  {
    xQueueReceive(xQueueRS485, rx_buffer, portMAX_DELAY);
    doAnswer = true;
    cmd = rx_buffer[RS485_CMD_OFFSET];
    arg_1 = rx_buffer[RS485_DATA_OFFSET];
    arg_2 = rx_buffer[RS485_DATA_OFFSET + 1];
    switch (cmd)
    {
      case CMD_SET_POS_100:                         // 0xDD command
        tx_buffer[0] = RSP_SET_POS_100;
        access_eeprom_bit = 0;                      // EEPROM writing denied
        if (!((arg_1 & 0b11100000) || (arg_2 & 0b10000000))) // bits 15, 14, 13 and 7 of the command argument must not be set!
        {
          tmp = arg_1;
          tmp = (tmp << 7) + arg_2;                 // Kearfott/Schiebel spec. here tmp: -100...+45 0x4A1...0xB60 or 1185..2912
          if (tmp > MOTOR_MAX_TP_KF)                // +90° for 0xDD com transformed according to Kearfott/Schiebel
            tmp = MOTOR_MAX_TP_KF;
          else if (tmp < MOTOR_MIN_TP_KF)           // -90° for 0xDD com transformed according to Kearfott/Schiebel
            tmp = MOTOR_MIN_TP_KF;
          GLOBAL_TargetPos = (tmp - 0x800) * 0.5933;   // ±200 0xFE00...0...0x0200
          //g_PosCmd = 1;                           // BLDC can be activate after powerup
        }
        tmp = GLOBAL_RealPos * 1.687 + 0x800;       // formula: 2048 + ((2048 - RealPos(magnet))*angular_ratio)
        // Transform Data for Position setting according to Schiebel spec.
        i = tmp >> 8;                               // Current position high byte
        i = (i << 1) & 0x1e;                        // high part of Shiebel argument
        if (tmp & 0x80)
          tx_buffer[2] = i + 1;                     // add bit7 as bit 0 to high byte (now high: 000bit11 bit10 bit9 bit8 bit7)
        else
          tx_buffer[2] = i;
        tx_buffer[3] = tmp & 0x7f;                  // now low byte:n 0 bit6 bit5 bit4 bit3 bit2 bit1 bit0
        //g_motor_extB = g_motor_extB_tmp;          //new motor power value (after power up: EEPROM power)
        break;

      case CMD_SET_POS_170:                         // 76 command
        access_eeprom_bit = 0;                      // EEPROM writing denied
        tmp = (arg_1 << 8) + arg_2;                 // desired position complete
        if (tmp < MOTOR_MIN_TP)
          tmp = MOTOR_MIN_TP;
        if (tmp > MOTOR_MAX_TP)
          tmp = MOTOR_MAX_TP;
        GLOBAL_TargetPos = tmp;
        tx_buffer[0] = RSP_SET_POS_170;
        //g_PosCmd = 1;                             // BLDC can be activate after powerup
        tx_buffer[2] = GLOBAL_RealPos >> 8;         // Current position high byte + decremented freshness counter
        tx_buffer[3] = GLOBAL_RealPos;              // Current position low byte
        //g_motor_extB = g_motor_extB_tmp;          //new motor power value (after power up: EEPROM power)
        break;

      case CMD_GET_ACT_POS_170:                     // 76 format compatible
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
          break;                                    // command wrong - if at least one of arguments > 0, no sens to send responsecode
        tx_buffer[RS485_CMD_OFFSET] = RSP_GET_ACT_POS_170;  // response code
        access_eeprom_bit = false;                  // EEPROM writing denied
        tx_buffer[RS485_DATA_OFFSET] = GLOBAL_RealPos >> 8;      // Current position high byte + decremented freshness counter
        tx_buffer[RS485_DATA_OFFSET + 1] = GLOBAL_RealPos;       // Current position low byte
        //g_motor_extB = g_motor_extB_tmp;            //new motor power value (after power up: EEPROM power)
        break;

     case CMD_SET_POS_170_SILENT:                   // 77 command
        access_eeprom_bit = false;                  // EEPROM writing denied
        tmp = (arg_1 << 8) + arg_2;                 // desired position complete
        if (tmp < MOTOR_MIN_TP)
          tmp = MOTOR_MIN_TP;
        if (tmp > MOTOR_MAX_TP)
          tmp = MOTOR_MAX_TP;
        GLOBAL_TargetPos = tmp;
        tx_buffer[0] = 0;
        //g_PosCmd = 1;                               // BLDC can be activate after powerup
        //g_motor_extB = g_motor_extB_tmp;            //new motor power value (after power up: EEPROM power)
        doAnswer = false;
        break;

     case CMD_SET_SCALED:                           // 78 command
        access_eeprom_bit = false;                  // EEPROM writing denied
        tmp = (arg_1 << 8) + arg_2;                 // desired position complete
        if (EE_Options & OPTIONS_REVERSE_ON)        // reverse rotation
            tmp = -tmp;
        if (tmp < MOTOR_MIN_TP)
          tmp = MOTOR_MIN_TP;
        if (tmp > MOTOR_MAX_TP)
          tmp = MOTOR_MAX_TP;
        /**< Do scaling */
        if (tmp > 0)                                // CCW scaling
            ltmp = (long)tmp * (long)EE_CCW_Scaling;
        else                                        // CW scaling
            ltmp = (long)tmp * (long)EE_CW_Scaling;
        tmp = ltmp/100;
        if (tmp < MOTOR_MIN_TP)
          tmp = MOTOR_MIN_TP;
        if (tmp > MOTOR_MAX_TP)
          tmp = MOTOR_MAX_TP;
        GLOBAL_TargetPos = tmp;
        tx_buffer[0] = RSP_SET_SCALED;
        //g_PosCmd = 1;                               // BLDC can be activate after powerup
        /**< Do "back scaling" */
        ltmp = (long)GLOBAL_RealPos * 100;
        if (ltmp > 0)                               // CCW back scaling
          ltmp = ltmp / (long)EE_CCW_Scaling;
        else                                        // CW back scaling
          ltmp = ltmp / (long)EE_CW_Scaling;
        tmp = (int)ltmp;
        if (EE_Options & OPTIONS_REVERSE_ON)        // reverse rotation
            tmp = -tmp;
        tx_buffer[2] = tmp >> 8;                    // Current position high byte + decremented freshness counter
        tx_buffer[3] = tmp;                         // Current position low byte
        //g_motor_extB = g_motor_extB_tmp;          //new motor power value (after power up: EEPROM power)
        break;

      case CMD_GET_ACT_POS_SCALED:                  // 76 format compatible
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
            break;                                  // command wrong - if at least one of arguments > 0, no sens to send responsecode
        tx_buffer[RS485_CMD_OFFSET] = RSP_GET_ACT_POS_SCALED;  // response code
        access_eeprom_bit = false;                  // EEPROM writing denied
        /**< "back scaling" */
        ltmp = (int32_t)GLOBAL_RealPos * 100;
        if (ltmp > 0)                             // CCW back scaling
          ltmp = ltmp / (int32_t)EE_CCW_Scaling;
        else                                      // CW back scaling
          ltmp = ltmp / (int32_t)EE_CW_Scaling;
        ltmp = ltmp / 100;
        tmp = (int16_t)ltmp;
        if (EE_Options & OPTIONS_REVERSE_ON)        // reverse rotation
          tmp = -tmp;
        tx_buffer[RS485_DATA_OFFSET] = tmp >> 8;    // Current position high byte + decremented freshness counter
        tx_buffer[RS485_DATA_OFFSET + 1] = tmp;     // Current position low byte
        //g_motor_extB = g_motor_extB_tmp;          //new motor power value (after power up: EEPROM power)
        break;

     case CMD_SET_AS_ZERO:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
            break;                                  // command wrong - if at least one of arguments > 0, no sens to send responsecode
        tx_buffer[0] = RSP_SET_AS_ZERO;             // response code
        //g_magStatus = MAGNET_Read();              // waiting for new magnet value after applying shift value of 0
        //Neutral = 2048 - g_magnet;                // saving deviation of standard neutral as new zero shift
        //g_TargetPos = 0;
        //g_magStatus = MAGNET_Read();              // waiting for new magnet value after applying shift value of 0
        tx_buffer[2] = GLOBAL_RealPos >> 8;         // Current position high byte
        tx_buffer[3] = GLOBAL_RealPos;              // Current position low byte
        break;

      case CMD_SET_ID:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 != arg_2)                         // check for validity
          break;                                    // command wrong (ID bytes are not same)
        if (arg_1 == 0 || arg_1 >= RS485_BROADCAST_ID)  // check for validity
          break;                                    // command wrong (ID bytes are not in range 0x01...0x1E)
        tx_buffer[RS485_CMD_OFFSET] = RSP_SET_ID;   // response code
        tx_buffer[RS485_DATA_OFFSET] = arg_1;       // data High = Station ID
        tx_buffer[RS485_DATA_OFFSET + 1] = arg_1;   // data Low = Station ID
        Station_ID = arg_1;                         // write station ID
        tx_buffer[RS485_ID_OFFSET] = arg_1;	        // set station ID
        break;

      case CMD_GET_ID:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity
          break;                                    // command wrong - if at least one of arguments > 0
        tx_buffer[RS485_CMD_OFFSET] = RSP_GET_ID;   // response code
        tx_buffer[RS485_DATA_OFFSET] = Station_ID;  // data High = Station ID
        tx_buffer[RS485_DATA_OFFSET + 1] = Station_ID; // data Low = Station ID
        break;

     case CMD_SET_FS_POS:
        access_eeprom_bit = false;                  // EEPROM writing denied
        tmp = (arg_1 << 8) + arg_2;
        if (tmp < MIN_TP || tmp > MAX_TP)           // checking value
          break;                                    // command wrong (position is out of range) - no sens to send Responsecode
        tx_buffer[0] = RSP_SET_FS_POS;
        EE_FailsafePos = tmp;                       // writing new value to EEPROM ???
        tx_buffer[2] = arg_1;
        tx_buffer[3] = arg_2;
        break;

     case CMD_GET_FS_POS:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if ( arg_1 || arg_2 )                       // check for validity (both must be 0)
          break;                                    // command wrong - if at least one of arguments > 0, no sens to send Responsecode
        tx_buffer[0] = RSP_GET_FS_POS;              // response code
        tx_buffer[2] = EE_FailsafePos >> 8;         // data High
        tx_buffer[3] = EE_FailsafePos;              // data Low
        break;

     case CMD_SET_AS_FAILSAFE:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
            break;                                  // command wrong - if at least one of arguments > 0, no sens to send Responsecode
        tx_buffer[0] = RSP_SET_AS_FAILSAFE;         // response code
        EE_FailsafePos = GLOBAL_RealPos;            // saving current position as new loss communication position ???
        tx_buffer[2] = GLOBAL_RealPos >> 8;         // Current position high byte
        tx_buffer[3] = GLOBAL_RealPos;              // Current position low byte
        break;

     case CMD_SET_FS_TIME:
        access_eeprom_bit = 0;                      // EEPROM writing denied
        if (arg_1 != arg_2)                         // check for validity
            break;                                  // command wrong (ID bytes are not same) - no sense to response
        if (arg_1 > 0x7F)                           // check for validity
            break;                                  // command wrong (ID bytes > 0x7F) - no sense to response
        tx_buffer[0] = RSP_SET_FS_TIME;             // response code
        tx_buffer[2] = arg_1;                       // data High
        tx_buffer[3] = arg_1;                       // data Low
        EE_FailsafeTime = arg_1;                    // write new value to EEPROM ???
        break;

     case CMD_GET_FS_TIME:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
            break;                                  // command wrong - if at least one of arguments > 0, no sens to send Responsecode
        tx_buffer[0] = RSP_GET_FS_TIME;             // response code
        tx_buffer[2] = EE_FailsafeTime;             // Loss of Communication timeout [*100ms]
        tx_buffer[3] = tx_buffer[2];                // same as Argument 1
        break;

      case CMD_VOLTAGE:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity (both must be 0)
          break;                                    // command wrong - if at least one of arguments > 0
        tx_buffer[RS485_CMD_OFFSET] = RSP_VOLTAGE;
        tx_buffer[RS485_DATA_OFFSET] = MONITOR_GetVoltage();
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[RS485_DATA_OFFSET];  // same as Argument 1
        break;

	    case CMD_CURRENT:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2)                         // check for validity
          break;                                    // command wrong - if at least one of arguments > 0
        tx_buffer[RS485_CMD_OFFSET] = RSP_CURRENT;    // response code
        tx_buffer[RS485_DATA_OFFSET] = MONITOR_GetCurrent();   // return current value
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[RS485_DATA_OFFSET];
        break;

	    case CMD_TEMPERATURE:
        access_eeprom_bit = false;                    // EEPROM writing denied
        tx_buffer[RS485_CMD_OFFSET] = RSP_TEMPERATURE;  // response code
        tx_buffer[RS485_DATA_OFFSET] = MONITOR_GetTemperature();
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[RS485_DATA_OFFSET];
        break;

      case CMD_RUNTIMER:
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1)
          break;
        tx_buffer[RS485_CMD_OFFSET] = RSP_RUNTIMER;
        ltmp = COUNTER_GetSeconds();
        if (arg_2 == 0x00)
        {
          tx_buffer[RS485_DATA_OFFSET] = (ltmp / 3600) >> 8;
          tx_buffer[RS485_DATA_OFFSET + 1] = (uint8_t)(ltmp / 3600);
        } else
        {
          tx_buffer[RS485_DATA_OFFSET] = (ltmp % 3600) / 60;
          tx_buffer[RS485_DATA_OFFSET + 1] = ltmp % 60;
        }
        break;

      case CMD_MAGNET:
        access_eeprom_bit = false;                  // EEPROM writing denied
        tx_buffer[RS485_CMD_OFFSET] = RSP_MAGNET;
        switch (arg_2)
        {
          case  1:
            tmp = GLOBAL_RealPos & 0xfff;
            tx_buffer[2] = tmp >> 8;      // production compensated magnet position
            tx_buffer[3] = tmp;           // production compensated magnet position
            break;
          /*case 2:
            tx_buffer[2]=0;                     // Hall sensor
            tx_buffer[3]=(PORTA.IN & 0xE0)>>5;  // hall sensor
            break;
          case 3:                                // magnet sensor status
            tx_buffer[2]=g_magStatus;
            tx_buffer[3]=g_magStatus;
            break;
          default:
            tx_buffer[2] = g_magnet >> 8; // absolute magnet position
            tx_buffer[3] = g_magnet;      // absolute magnet position
            break;*/
         }
         break;

      case CMD_READ_SERIAL_NUM:
      case CMD_READ_PROD_DESCR:
      case CMD_READ_FIRMWARE_NUM:
      case CMD_READ_HWREV_NUM:
        switch (cmd)
        {
          case CMD_READ_SERIAL_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_READ_SERIAL_NUM;         // response code
            tmp = EEPROM_ESN_ADDRESS;
            break;
          case CMD_READ_PROD_DESCR:
            tx_buffer[RS485_CMD_OFFSET] = RSP_READ_PROD_DESCR;         // response code
            tmp = EEPROM_PROD_ADDRESS;
            break;
          case CMD_READ_FIRMWARE_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_READ_FIRMWARE_NUM;       // response code
            tmp = EEPROM_FWREV_ADDRESS;
            break;
          case CMD_READ_HWREV_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_READ_HWREV_NUM;          // response code
            tmp = EEPROM_HWREV_ADDRESS;
            break;
        }
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 || arg_2 > EEPROM_LEN_STRING)     // check for validity
          break;                                    // command wrong (Argument1 must be 0, Arg2 = 0..0x20)
        for (i = 0; i < EEPROM_LEN_STRING; i++)     // calculate number of characters
        {
          if (EEPROM_ReadByte(tmp + i) == 0)
            break;
        }
        tx_buffer[RS485_DATA_OFFSET] = EEPROM_ReadByte(tmp + arg_2);
        tx_buffer[RS485_DATA_OFFSET + 1] = i;       // data Low (Length of serial number string)
        break;

      case CMD_READ_MP:                             // read Motor Parameters (EEPROM address range 0x20...0x27)
        access_eeprom_bit = false;                  // EEPROM writing denied
        if (arg_1 < 0x20 || arg_1 > 0x30)
          break;                                    // wrong EEPROM address range
        tx_buffer[RS485_CMD_OFFSET] = RSP_READ_MP;
        tx_buffer[RS485_DATA_OFFSET] = EEPROM_ReadByte((uint16_t)arg_2);  // data
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[RS485_DATA_OFFSET];
        break;

      case CMD_WRITE_MP:                            // read Motor Parameters (EEPROM address range 0x20...0x27)
        if (access_eeprom_bit == false || arg_1 < 0x20 || arg_1 > 0x30)
          break;                                    // access to EEPROM denied
        //EEPROM_WriteByteSecure((uint16_t)arg_1, arg_2);
        EEPROM_WriteByte((uint16_t)arg_1, arg_2);
        tx_buffer[RS485_CMD_OFFSET] = RSP_WRITE_MP;
        tx_buffer[RS485_DATA_OFFSET] = arg_1;       // data High = 8bit address (0x20...0x27)
        tx_buffer[RS485_DATA_OFFSET + 1] = arg_2;   // data Low  = Value
        break;

      case CMD_WRITE_SERIAL_NUM:
      case CMD_WRITE_PROD_DESCR:
      case CMD_WRITE_FIRMWARE_NUM:
      case CMD_WRITE_HWREV_NUM:
        switch (cmd)
        {
          case CMD_WRITE_SERIAL_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_WRITE_SERIAL_NUM;
            tmp = EEPROM_ESN_ADDRESS;
            break;
          case CMD_WRITE_PROD_DESCR:
            tx_buffer[RS485_CMD_OFFSET] = RSP_WRITE_PROD_DESCR;
            tmp = EEPROM_PROD_ADDRESS;
            break;
          case CMD_WRITE_FIRMWARE_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_WRITE_FIRMWARE_NUM;
            tmp = EEPROM_FWREV_ADDRESS;
            break;
          case CMD_WRITE_HWREV_NUM:
            tx_buffer[RS485_CMD_OFFSET] = RSP_WRITE_HWREV_NUM;
            tmp = EEPROM_HWREV_ADDRESS;
            break;
        }
        if (arg_2 > EEPROM_LEN_STRING || access_eeprom_bit == false)
          break;                                      // command wrong
        tx_buffer[RS485_DATA_OFFSET] = arg_1;
        tx_buffer[RS485_DATA_OFFSET + 1] = arg_2;
        EEPROM_WriteByte(tmp + arg_2, arg_1);
        break;

      case CMD_RD_EEPROM_LOW:                       // read any low 0x00...0xFF EEPROM cell
        access_eeprom_bit = false;                  // EEPROM writing denied
        tx_buffer[RS485_CMD_OFFSET] = RSP_RD_EEPROM_LOW;
        tx_buffer[RS485_DATA_OFFSET] = EEPROM_ReadByte((uint16_t)arg_2);
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[RS485_DATA_OFFSET];
        break;

      case CMD_WR_EEPROM_LOW:                       // write any low 0x00...0xFF EEPROM cell
        if (!access_eeprom_bit)
          break;                                    // access to EEPROM denied
        //EEPROM_WriteByteSecure((uint16_t)arg_1, arg_2);
        EEPROM_WriteByte((uint16_t)arg_1, arg_2);
        tx_buffer[RS485_CMD_OFFSET] = RSP_WR_EEPROM_LOW;
        tx_buffer[RS485_DATA_OFFSET] = arg_1;       // data High = 8bit address
        tx_buffer[RS485_DATA_OFFSET + 1] = arg_2;   // data Low  = Value
        break;

      case CMD_RD_EEPROM_HIGH:
        tx_buffer[RS485_CMD_OFFSET] = RSP_RD_EEPROM_HIGH;
        access_eeprom_bit = false;                      // EEPROM writing denied
        tx_buffer[RS485_DATA_OFFSET] = EEPROM_ReadByte((uint16_t)arg_2 + 0x100);
        tx_buffer[RS485_DATA_OFFSET + 1] = tx_buffer[2];  // data Low
        break;

      case CMD_WR_EEPROM_HIGH:                      // write any high 0x0100...0x01FF EEPROM cell
        if (access_eeprom_bit == false)
          break;                                    // access to EEPROM denied - no sens to send responsecode
        //EEPROM_WriteByteSecure((uint16_t)arg_1, arg_2);
        EEPROM_WriteByte((uint16_t)arg_1, arg_2);
        tx_buffer[RS485_CMD_OFFSET] = RSP_WR_EEPROM_HIGH;
        tx_buffer[RS485_DATA_OFFSET] = arg_1;       // 8bit address
        tx_buffer[RS485_DATA_OFFSET + 1] = arg_2;   // data
        break;

      case 0xFF:
        //access bit section
        if (arg_1 == 0x41 && arg_2 == 0x3F)
        {
          access_eeprom_bit = true; // access to EEPROM is enabled
          tx_buffer[0] = 0xFF;
          tx_buffer[2] = 'A';                       // 'A'ccess
          tx_buffer[3] = 'E';                       // 'E'nable
        } else
        if (arg_1 == 'R' && arg_2 == 'R' && access_eeprom_bit == true)                // 'R'eset 'R'equest
        {
          MOTOR_Off();                              // stop motor
          __disable_irq();                          // Global interrupts disable
          while (1);                                // endless loop - waiting for watchdog reset
        }
        break;

      default:
        doAnswer = false;
        break;
    }

    if (doAnswer)
    {
      crc = CRC16_CalcKearfott(tx_buffer, RS485_PACKET_LEN - RS485_CRC_LEN);
      tx_buffer[RS485_CRC_OFFSET] = (uint8_t)(crc >> 8);
      tx_buffer[RS485_CRC_OFFSET + 1] = (uint8_t)(crc & 0xff);
      RS485_SendPacket(tx_buffer, RS485_PACKET_LEN);
    }
  }
}
