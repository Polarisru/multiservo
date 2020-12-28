#include "driver_gpio.h"
#include "driver_uart.h"
#include "crc16.h"
#include "cmd_set.h"
#include "defines.h"
#include "global.h"
#include "rs485.h"

#define RS485_PORT          GPIO_PORTA
#define RS485_PIN_DIR       10
#define RS485_PIN_TX        8
#define RS485_PIN_RX        9
#define RS485_PINMUX_TX     MUX_PA08D_SERCOM2_PAD0
#define RS485_PINMUX_RX     MUX_PA09D_SERCOM2_PAD1

#define RS485_RXPO          1
#define RS485_TXPO          0

#define RS485_CHANNEL       SERCOM2
#define RS485_INTHANDLER    SERCOM2_Handler
#define RS485_IRQ           SERCOM2_IRQn

#define RS485_BAUDRATE      115200

#define RS485_MAX_RETRIES   3

#define fixed_ID            0x01
#define RS485_BROADCAST_ID  0x1F

uint8_t Station_ID;
/**< Buffer for received data */
uint8_t RS485_RxData[RS485_PAKET_SIZE];

/** \brief Interrupt handler to process RS485 messages
 *
 * \return Nothing
 *
 */
void RS485_INTHANDLER(void)
{
  uint8_t ch, i;
  uint16_t crc;
  static uint8_t buffer[RS485_PAKET_SIZE] = {0};
  BaseType_t xHigherPriorityTaskWoken = pdFALSE;

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
		for (i = 0; i < RS485_PAKET_SIZE - 1; i++)
      buffer[i] = buffer[i + 1];
    buffer[RS485_PAKET_SIZE - 1] = ch;
    if ((buffer[RS485_OFFS_ID] == Station_ID) || (buffer[RS485_OFFS_ID] == RS485_BROADCAST_ID))
    {
      /**< have got our ID or broadcast ID */
      crc = CRC16_INIT_VAL;
      crc = CRC16_Calc(crc, buffer, RS485_PAKET_SIZE - RS485_CRC_LEN);
      if ((uint8_t)(crc >> 8) == buffer[RS485_OFFS_CRC] && (uint8_t)crc == buffer[RS485_OFFS_CRC + 1])
      {
        /**< CRC is Ok, place packet to RS485 processing queue */
        memcpy(RS485_RxData, buffer, RS485_OFFS_CRC);
        xEventGroupSetBitsFromISR(xEventGroupCommon, EG_BIT_RS485_RX, &xHigherPriorityTaskWoken);
      }
    }
  }

  /**< Now the buffer is empty we can switch context if necessary */
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/** \brief Set RS485 baudrate
 *
 * \param [in] baudrate New baudrate value
 * \return Nothing
 *
 */
void RS485_SetBaudrate(uint32_t baudrate)
{
  UART_SetBaudrate(RS485_CHANNEL, baudrate);
}

/** \brief Enable Rx interrupt for RS485 communication
 *
 * \return Nothing
 *
 */
void RS485_EnableRxInt(void)
{
  RS485_CHANNEL->USART.INTFLAG.reg = 0;
  RS485_CHANNEL->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
}

/** \brief Disable Rx interrupt for RS485 communication
 *
 * \return Nothing
 *
 */
void RS485_DisableRxInt(void)
{
  RS485_CHANNEL->USART.INTENSET.reg = 0;
}

/** \brief Send data packet via RS485 bus
 *
 * \param [in] data Buffer with data to send
 * \param [in] len Length of data packet
 * \return Nothing
 *
 */
void RS485_Send(uint8_t *data, uint8_t len)
{
  RS485_CHANNEL->USART.CTRLB.bit.RXEN = 0;
  GPIO_SetPin(RS485_PORT, RS485_PIN_DIR);

  while (len--)
    UART_SendByte(RS485_CHANNEL, *data++);

  GPIO_ClearPin(RS485_PORT, RS485_PIN_DIR);
  RS485_CHANNEL->USART.CTRLB.bit.RXEN = 1;
}

/** \brief Receive data from RS485 (without interrupt)
 *
 * \param [out] data Pointer to data buffer
 * \param [in] len Length of data to receive
 * \param [in] timeout Timeout to receive data in system ticks (1ms)
 * \return True if succeed
 *
 */
bool RS485_Receive(uint8_t *data, uint16_t len, uint16_t timeout)
{
  uint32_t ticks = xTaskGetTickCount();

  while (len--)
  {
    /**< Wait until the data is ready to be received */
    while ((RS485_CHANNEL->USART.INTFLAG.bit.RXC == 0) && ((xTaskGetTickCount() - ticks) < timeout));
    /**< Read RX data, combine with DR mask */
    *data++ = (uint8_t)(RS485_CHANNEL->USART.DATA.reg & 0xFF);
  }

  return true;
}

/** \brief Send RS485 packet and wait for reply if needed
 *
 * \param [in] tx_data Data buffer with packet to send
 * \param [out] rx_data Buffer with reply data
 * \param [in] timeout Timeout value in ms
 * \return true if reply was received
 *
 */
bool RS485_Transfer(uint8_t *tx_data, uint8_t reply, uint8_t *rx_data, uint8_t timeout)
{
  EventBits_t uxBits;
  uint8_t errors = 0;
  uint16_t crc;

  while (errors < RS485_MAX_RETRIES)
  {
    crc = CRC16_INIT_VAL;
    crc = CRC16_Calc(crc, tx_data, RS485_OFFS_CRC);
    tx_data[RS485_OFFS_CRC] = (uint8_t)(crc >> 8);
    tx_data[RS485_OFFS_CRC + 1] = (uint8_t)crc;
    RS485_Send(tx_data, RS485_PAKET_SIZE);
    if (rx_data == NULL)
      return true;

    /**< Wait till reply is received or timeout occurred */
    uxBits = xEventGroupWaitBits(xEventGroupCommon, EG_BIT_RS485_RX, pdTRUE, pdFALSE, timeout);
    if (((uxBits & EG_BIT_RS485_RX) == EG_BIT_RS485_RX) && (RS485_RxData[RS485_OFFS_CMD] == reply))
    {
      memcpy(rx_data, RS485_RxData, RS485_OFFS_CRC);
      return true;
    }

    errors++;
  }

  return false;
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
  GPIO_SetFunction(RS485_PORT, RS485_PIN_TX, RS485_PINMUX_TX);
  GPIO_SetFunction(RS485_PORT, RS485_PIN_RX, RS485_PINMUX_RX);

  /**< enable receiving interrupt */
  RS485_CHANNEL->USART.INTENSET.reg = SERCOM_USART_INTFLAG_RXC;
  /**< Should use lower priority than FreeRTOS interrupts to allow interrupts usage for the RTOS */
  NVIC_SetPriority(RS485_IRQ, 7);
  NVIC_EnableIRQ(RS485_IRQ);

  Station_ID = fixed_ID;
}
