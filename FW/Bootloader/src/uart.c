#include "uart.h"

/** \brief Send one char to serial interface
 *
 * \param [in] c Value to send
 * \return Nothing
 *
 */
void UART_SendChar(uint8_t c)
{
  UART_NUM->DR = c;

  /**< Wait for end of transmission */
  while ((UART_NUM->SR & USART_SR_TXE) == 0);
}

/** \brief Receive one byte from serial interface
 *
 * \return Received byte as uint8_t
 *
 */
uint8_t UART_GetChar(void)
{
  /**< Wait until the data is ready to be received */
  while ((UART_NUM->SR & USART_SR_RXNE) == 0);

  /**< Read RX data, combine with DR mask */
  return (uint8_t)(UART_NUM->DR & 0xFF);
}

/** \brief Check if any data is present in serial input buffer
 *
 * \return True if buffer has data
 *
 */
bool UART_HaveData(void)
{
  if ((UART_NUM->SR & USART_SR_RXNE) != 0)
    return 1;
  else
    return 0;
}

/** \brief Initialize UART module for communication with PC
 *
 * \return Nothing
 *
 */
void UART_Configuration(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef  USART_InitStructure;

  UART_CLOCK_ENABLE;

  /**< Configure USART Tx and Rx as alternate function */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = UART_TX_PIN | UART_RX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(UART_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(UART_GPIO, UART_RX_SOURCE, UART_GPIO_AF);
  GPIO_PinAFConfig(UART_GPIO, UART_TX_SOURCE, UART_GPIO_AF);

  USART_InitStructure.USART_BaudRate = UART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(UART_NUM, &USART_InitStructure);

  /**< Start UART interface */
  USART_Cmd(UART_NUM, ENABLE);
}
