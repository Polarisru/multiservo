#include "driver_gpio.h"
#include "driver_uart.h"
#include "rs232.h"

#define RS232_PORT          GPIO_PORTA
#define RS232_PIN_TX        4
#define RS232_PIN_RX        5
#define RS232_PINMUX_TX     MUX_PA04D_SERCOM0_PAD0
#define RS232_PINMUX_RX     MUX_PA05D_SERCOM0_PAD1

#define RS232_RXPO          1
#define RS232_TXPO          0

#define RS232_CHANNEL       SERCOM0

#define RS232_BAUDRATE	    115200L

/** \brief Send one char to serial interface
 *
 * \param [in] c Value to send
 * \return Nothing
 *
 */
void RS232_SendChar(uint8_t c)
{
  UART_SendByte(RS232_CHANNEL, c);
}

/** \brief Receive one byte from serial interface
 *
 * \return Received byte as uint8_t
 *
 */
uint8_t RS232_GetChar(void)
{
  return UART_GetByte(RS232_CHANNEL);
}

/** \brief Check if any data is present in serial input buffer
 *
 * \return True if buffer has data
 *
 */
bool RS232_HaveData(void)
{
  return UART_HaveData(RS232_CHANNEL);
}

/** \brief Disable UART
 *
 * \return Nothing
 *
 */
void RS232_Disable(void)
{
  UART_DeInit(RS232_CHANNEL);
}

/** \brief Initialize UART module for communication with PC
 *
 * \return Nothing
 *
 */
void RS232_Configuration(void)
{
  /**< Configure UART for RS232 bus */
  UART_Init(RS232_CHANNEL, RS232_RXPO, RS232_TXPO, RS232_BAUDRATE, false);

  /**< Setup TX/RX pins */
  GPIO_SetFunction(RS232_PORT, RS232_PIN_TX, RS232_PINMUX_TX);
  GPIO_SetFunction(RS232_PORT, RS232_PIN_RX, RS232_PINMUX_RX);
}
