#include "driver_gpio.h"
#include "driver_uart.h"
#include "crc16.h"
#include "cmd_set.h"
#include "defines.h"
#include "global.h"
#include "rs485.h"

#define RS485_PORT          GPIO_PORTA
#define RS485_PIN_DIR       23
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

/** \brief Interrupt handler to process RS485 messages
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

void RS485_SetBaudrate(uint32_t baudrate)
{

}

/** \brief Initialize RS485 module
 *
 * \return Nothing
 *
 */
void RS485_Configuration(void)
{
  /**< Configure UART for RS485 bus */
  UART_Init(RS485_CHANNEL, RS485_RXPO, RS485_TXPO, RS485_BAUDRATE, false);

  /**< configure DIR pin and TE pin */
  GPIO_ClearPin(RS485_PORT, RS485_PIN_DIR);
  GPIO_SetDir(RS485_PORT, RS485_PIN_DIR, true);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_DIR, GPIO_PIN_FUNC_OFF);
  /**< Setup TX/RX pins */
  GPIO_SetFunction(RS485_PORT, RS485_PIN_TX, MUX_PA24C_SERCOM3_PAD2);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_RX, MUX_PA25C_SERCOM3_PAD3);

  /**< enable receiving interrupt */
  RS485_CHANNEL->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
  NVIC_EnableIRQ(SERCOM3_IRQn);
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
