#ifndef _SBUS_H
#define _SBUS_H

#include "defines.h"
#include "stm32f30x_usart.h"

#define SBUS_BAUDRATE           100000UL

#define SBUS_UART_NUM           USART1
#define SBUS_CLOCK_ENABLE       RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE)
#define SBUS_GPIO               GPIOB

#define SBUS_TX_SOURCE          GPIO_PinSource6
#define SBUS_RX_SOURCE          GPIO_PinSource7

#define SBUS_TX_PIN             (1 << SBUS_TX_SOURCE)
#define SBUS_RX_PIN             (1 << SBUS_RX_SOURCE)

#define SBUS_GPIO_AF            GPIO_AF_7

#define SBUS_CMD_START          0xF0
#define SBUS_CMD_STOP           0x00
#define SBUS_CMD_PROG           0xF9

#define SBUS_MAX_CHANNEL        16

#define SBUS_DATA_BITS          11
#define SBUS_VALUE_MASK         ((1 << SBUS_DATA_BITS) - 1)

#define SBUS_OFFS_CMD           0
#define SBUS_OFFS_DATA          1
#define SBUS_OFFS_FLAGS         23

#define SBUS_DATA_LEN           25
#define SBUS_PROG_LEN           17
#define SBUS_MAX_DATALEN        SBUS_DATA_LEN

void SBUS_Send(uint8_t *data, uint8_t len);
void SBUS_SendCmd(void);
void SBUS_Config(void);

#endif
