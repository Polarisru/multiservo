#include "sbus.h"

static uint16_t SBUS_Values[SBUS_MAX_CHANNEL] = {0};
static uint8_t SBUS_Data[SBUS_MAX_DATALEN];
static uint8_t SBUS_Flags = 0;

void SBUS_Send(uint8_t *data, uint8_t len)
{
  while (len-- > 0)
  {
    //while (USART_GetFlagStatus(SBUS_UART_NUM, USART_FLAG_TC) == RESET);
    //USART_SendData(SBUS_UART_NUM, *data++);
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

void SBUS_Config(void)
{
//  GPIO_InitTypeDef  GPIO_InitStructure;
//  USART_InitTypeDef USART_InitStructure;
//
//  SBUS_CLOCK_ENABLE;
//
//  /**< Setup UART pins (Rx and Tx) */
//  GPIO_InitStructure.GPIO_Pin = SBUS_TX_PIN;// | SBUS_RX_PIN;
//  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//  GPIO_Init(SBUS_GPIO, &GPIO_InitStructure);
//
//  GPIO_PinAFConfig(SBUS_GPIO, SBUS_TX_SOURCE, SBUS_GPIO_AF);
//  //GPIO_PinAFConfig(SBUS_GPIO, SBUS_RX_SOURCE, SBUS_GPIO_AF);
//
//  /**< Configure UART for SBUS */
//  USART_DeInit(SBUS_UART_NUM);
//  USART_InitStructure.USART_BaudRate            = SBUS_BAUDRATE;
//  USART_InitStructure.USART_WordLength          = USART_WordLength_9b;
//  USART_InitStructure.USART_StopBits            = USART_StopBits_2;
//  USART_InitStructure.USART_Parity              = USART_Parity_Even;
//  USART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;
//  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//  USART_Init(SBUS_UART_NUM, &USART_InitStructure);
//
//  /**< Enable the USART's most significant bit first */
//  USART_MSBFirstCmd(SBUS_UART_NUM, ENABLE);
//  /**< Enable the Pin active level inversion (for both Rx and Tx) */
//  USART_InvPinCmd(SBUS_UART_NUM, USART_InvPin_Tx, ENABLE);
//  USART_InvPinCmd(SBUS_UART_NUM, USART_InvPin_Rx, ENABLE);
//
//  /**< Enable UART for SBUS */
//  USART_Cmd(SBUS_UART_NUM, ENABLE);
}
