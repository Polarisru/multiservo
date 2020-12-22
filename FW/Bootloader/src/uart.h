#ifndef _UART_H
#define _UART_H

#include "defines.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_usart.h"

#define UART_BUFFER_SIZE				(256)

#define UART_BAUDRATE	        	(115200L)

#define UART_NUM                USART2

#define UART_CLOCK_ENABLE       RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE)

#define UART_GPIO             	GPIOA
#define UART_RX_SOURCE          GPIO_PinSource3
#define UART_TX_SOURCE          GPIO_PinSource2

#define UART_GPIO_AF            GPIO_AF_USART2

#define UART_RX_PIN            	(1 << UART_RX_SOURCE)
#define UART_TX_PIN            	(1 << UART_TX_SOURCE)

//#define UART_NUM                  UART5
//
//#define UART_CLOCK_ENABLE         RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE)
//
//#define UART_GPIO             	  GPIOE
//#define UART_RX_SOURCE            GPIO_PinSource7
//#define UART_TX_SOURCE            GPIO_PinSource8
//#define USART1_IRQn              	USART5_IRQn
//#define USART1_IRQHandler        	USART5_IRQHandler
//
//#define UART_GPIO_AF              GPIO_AF_UART5
//
//#define UART_RX_PIN            	  (1 << UART_RX_SOURCE)
//#define UART_TX_PIN            	  (1 << UART_TX_SOURCE)


void UART_SendChar(uint8_t c);
uint8_t UART_GetChar(void);
bool UART_HaveData(void);
void UART_Configuration(void);

#endif
