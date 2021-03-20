#include "drivers.h"
#include "outputs.h"

#define SERIAL_PORT          GPIO_PORTA
#define SERIAL_PIN_TX        0
#define SERIAL_PIN_RX        1
#define SERIAL_PINMUX_TX     MUX_PA00D_SERCOM1_PAD0
#define SERIAL_PINMUX_RX     MUX_PA01D_SERCOM1_PAD1

#define SERIAL_RXPO          1
#define SERIAL_TXPO          0

#define SERIAL_CHANNEL       SERCOM1
#define SERIAL_INTHANDLER    SERCOM1_Handler
#define SERIAL_IRQ           SERCOM1_IRQn

#define SERIAL_BAUDRATE      100000UL

/** \brief Enable serial connection
 *
 * \return void
 *
 */
void SERIAL_Enable(void)
{
  /**< Setup TX/RX pins */
  GPIO_SetFunction(SERIAL_PORT, SERIAL_PIN_TX, SERIAL_PINMUX_TX);
  GPIO_SetFunction(SERIAL_PORT, SERIAL_PIN_RX, SERIAL_PINMUX_RX);
}

/** \brief Disable serial connection
 *
 * \return void
 *
 */
void SERIAL_Disable(void)
{
  /**< Disable pins */
  GPIO_SetFunction(SERIAL_PORT, SERIAL_PIN_TX, GPIO_PIN_FUNC_OFF);
  GPIO_SetFunction(SERIAL_PORT, SERIAL_PIN_RX, GPIO_PIN_FUNC_OFF);
}

/** \brief Set baudrate for serial connection
 *
 * \param [in] baudrate New baudrate value
 * \return void
 *
 */
void SERIAL_SetBaudrate(uint32_t baudrate)
{
  UART_SetBaudrate(SERIAL_CHANNEL, baudrate);
}

/** \brief Send data to serial channel
 *
 * \param [in] data Pointer to data buffer
 * \param [in] len Data length
 * \return void
 *
 */
void SERIAL_Send(uint8_t *data, uint16_t len)
{
  while (len--)
    UART_SendByte(SERIAL_PORT, *data++);
}

/** \brief Receive data from serial channel
 *
 * \param [out] data Pointer to data buffer
 * \param [in] len Length of data to receive
 * \param [in] timeout Timeout to receive data in system ticks (1ms)
 * \return True if succeed
 *
 */
bool SERIAL_Receive(uint8_t *data, uint16_t len, uint16_t timeout)
{
  uint32_t ticks = xTaskGetTickCount() + timeout;

  while (len > 0)
  {
    if (UART_HaveData(SERIAL_PORT) == true)
    {
      *data++ = UART_GetByte(SERIAL_PORT);
      len--;
    }
    if (xTaskGetTickCount() > ticks)
      return false;
  }
  return true;
}

/** \brief Configure serial connection
 *
 * \param void
 * \return void
 *
 */
void SERIAL_Configuration(void)
{
  /**< Configure UART for serial connection */
  UART_Init(SERIAL_CHANNEL, SERIAL_RXPO, SERIAL_TXPO, SERIAL_BAUDRATE, USART_PARITY_EVEN, true);
}

