#include "driver_gpio.h"
#include "driver_uart.h"
#include "outputs.h"
#include "sbus.h"

#define SBUS_PORT          GPIO_PORTA
#define SBUS_PIN_TX        0
#define SBUS_PIN_RX        1
#define SBUS_PINMUX_TX     MUX_PA00D_SERCOM1_PAD0
#define SBUS_PINMUX_RX     MUX_PA01D_SERCOM1_PAD1

#define SBUS_RXPO          1
#define SBUS_TXPO          0

#define SBUS_CHANNEL       SERCOM1
#define SBUS_INTHANDLER    SERCOM1_Handler
#define SBUS_IRQ           SERCOM1_IRQn

#define SBUS_BAUDRATE      100000UL

static uint16_t SBUS_Values[SBUS_MAX_CHANNEL] = {0};
static uint8_t SBUS_Data[SBUS_MAX_DATALEN];
static uint8_t SBUS_Flags = 0;

void SBUS_Send(uint8_t *data, uint8_t len)
{
  while (len-- > 0)
  {
    UART_SendByte(SBUS_CHANNEL, *data++);
  }
}

void SBUS_SetChannel(uint8_t channel, uint16_t value)
{
  if (channel >= SBUS_MAX_CHANNEL)
    return;

  SBUS_Values[channel] = value & SBUS_VALUE_MASK;
}

void SBUS_SendCmd(void)
{
  uint8_t i;
  //uint8_t rest;
  uint8_t len = SBUS_DATA_LEN;
  //uint16_t pos;

  memset(SBUS_Data, 0, SBUS_DATA_LEN);
  SBUS_Data[SBUS_OFFS_CMD] = SBUS_CMD_START;
  for (i = 0; i < SBUS_MAX_CHANNEL * SBUS_DATA_BITS; i++)
  {
    if (SBUS_Values[i / SBUS_DATA_BITS] & (i % SBUS_DATA_BITS))
      SBUS_Data[SBUS_OFFS_DATA + (i / 8)] |= (1 << (i % 8));
  }
  SBUS_Data[SBUS_OFFS_FLAGS] = SBUS_Flags;
  SBUS_Data[len - 1] = SBUS_CMD_STOP;

  SBUS_Send(SBUS_Data, len);
}

void SBUS_Enable(void)
{
  /**< Setup TX/RX pins */
  GPIO_SetFunction(SBUS_PORT, SBUS_PIN_TX, SBUS_PINMUX_TX);
  GPIO_SetFunction(SBUS_PORT, SBUS_PIN_RX, SBUS_PINMUX_RX);
  OUTPUTS_Switch(OUTPUTS_SBUSPOL, OUTPUTS_SWITCH_ON);
}

void SBUS_Disable(void)
{
  GPIO_SetFunction(SBUS_PORT, SBUS_PIN_TX, GPIO_PIN_FUNC_OFF);
  GPIO_SetFunction(SBUS_PORT, SBUS_PIN_RX, GPIO_PIN_FUNC_OFF);
  OUTPUTS_Switch(OUTPUTS_SBUSPOL, OUTPUTS_SWITCH_OFF);
}

void SBUS_Configuration(void)
{
  /**< Configure UART for SBUS */
  UART_Init(SBUS_CHANNEL, SBUS_RXPO, SBUS_TXPO, SBUS_BAUDRATE, false);
}
