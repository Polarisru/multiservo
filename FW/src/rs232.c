#include "global.h"
#include "rs232.h"

#define RS232_PORT          GPIO_PORTA
#define RS232_PIN_TX        4
#define RS232_PIN_RX        5
#define RS232_PINMUX_TX     MUX_PA04D_SERCOM0_PAD0
#define RS232_PINMUX_RX     MUX_PA05D_SERCOM0_PAD1

#define RS232_RXPO          1
#define RS232_TXPO          0

#define RS232_CHANNEL       SERCOM0
#define RS232_INTHANDLER    SERCOM0_Handler
#define RS232_IRQ           SERCOM0_IRQn

#define RS232_BAUDRATE      115200

/**< Receiving buffer for RS232 communication */
char RS232_RxBuffer[RS232_BUFFER_SIZE];
uint8_t RS232_RxHead;

/** \brief Interrupt handler to process RS232 messages
 *
 * \return Nothing
 *
 */
void RS232_INTHANDLER(void)
{
  uint8_t ch;
  BaseType_t xHigherPriorityTaskWoken;

  /**< We have not woken a task at the start of the ISR */
  xHigherPriorityTaskWoken = pdFALSE;

  if ((RS232_CHANNEL->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) >> SERCOM_USART_INTFLAG_RXC_Pos)
  {
    if (RS232_CHANNEL->USART.STATUS.reg & (SERCOM_USART_STATUS_PERR | SERCOM_USART_STATUS_FERR |
                                     SERCOM_USART_STATUS_BUFOVF | SERCOM_USART_STATUS_ISF | SERCOM_USART_STATUS_COLL))
    {
      RS232_CHANNEL->USART.STATUS.reg = SERCOM_USART_STATUS_MASK;
			return;
		}

    ch = RS232_CHANNEL->USART.DATA.reg & 0x7F;

    if (ch == CHAR_CR)
      return;

    RS232_RxBuffer[RS232_RxHead++] = ch;

    if (ch == CHAR_ESC)
    {
      RS232_RxHead = 0;
      return;
    }

    if (ch == CHAR_NL)
    {
      RS232_RxBuffer[RS232_RxHead] = 0;
      RS232_RxHead = 0;
      vTaskNotifyGiveFromISR(xTaskComm, &xHigherPriorityTaskWoken);
    }
  }

  /**< Now the buffer is empty we can switch context if necessary */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/** \brief Send data packet to RS232
 *
 * \param [in] data Pointer to data to send
 * \param [in] len Length of data
 * \return Nothing
 *
 */
void RS232_SendData(uint8_t *data, uint8_t len)
{
  while (len--)
    UART_SendByte(RS232_CHANNEL, *data++);
}

/** \brief Reset Rx queue
 *
 * \return Nothing
 *
 */
void RS232_ResetRx(void)
{
  RS232_RxHead = 0;
}

/** \brief Enable Rx interrupt for RS232
 *
 * \return void
 *
 */
void RS232_EnableRxInt(void)
{
  NVIC_EnableIRQ(RS232_IRQ);
}

/** \brief Disable Rx interrupt for RS232
 *
 * \return void
 *
 */
void RS232_DisableRxInt(void)
{
  NVIC_DisableIRQ(RS232_IRQ);
}

/** \brief Receive data from RS232 to buffer
 *
 * \param [out] data Pointer to data buffer
 * \param [in] len Length of data
 * \param [in] timeout Timeout in ms
 * \return True if succeed
 *
 */
bool RS232_Receive(uint8_t *data, uint16_t len, uint16_t timeout)
{
  uint32_t ticks = xTaskGetTickCount() + timeout;

  while (len > 0)
  {
    if (UART_HaveData(RS232_CHANNEL) == true)
    {
      *data++ = UART_GetByte(RS232_CHANNEL);
      len--;
    }
    if (xTaskGetTickCount() > ticks)
      return false;
  }
  return true;
}

/** \brief Set RS232 baudrate
 *
 * \param [in] baudrate Baudrate value to set
 * \return Nothing
 *
 */
void RS232_SetBaudrate(uint32_t baudrate)
{
  UART_SetBaudrate(RS232_CHANNEL, baudrate);
}

void RS232_Configuration(void)
{
  /**< Configure UART for RS232 bus */
  UART_Init(RS232_CHANNEL, RS232_RXPO, RS232_TXPO, RS232_BAUDRATE, USART_PARITY_NONE, false);

  /**< Setup TX/RX pins */
  GPIO_SetFunction(RS232_PORT, RS232_PIN_TX, RS232_PINMUX_TX);
  GPIO_SetFunction(RS232_PORT, RS232_PIN_RX, RS232_PINMUX_RX);

  /**< enable receiving interrupt */
  RS232_CHANNEL->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
  NVIC_EnableIRQ(RS232_IRQ);
}
