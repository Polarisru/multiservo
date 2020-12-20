#ifndef _BOOTLOADER_H
#define _BOOTLOADER_H

#include "defines.h"

/**< Settings for PWM bootloader */
#define BL_PWM_RETRY          5
#define BL_PWM_RESYNCS        20

#define BL_PAGE_SIZE          256

#define BL_FLASH_PAGE_SEC_L	  0x80	// Write to FLASH 128byte Format: COM Page# DATA(128 bytes). Total 130 bytes. Secured
#define BL_FLASH_PAGE_SEC_H	  0x81	// Write to FLASH 128byte Format: COM Page# DATA(128 bytes). Total 130 bytes. Secured
#define BL_FLASH_PAGE_L	      0x82	// Write to FLASH 128byte Format: COM Page# DATA(128 bytes). Total 130 bytes. Unsecured
#define BL_FLASH_PAGE_H	      0x83	// Write to FLASH 128byte Format: COM Page# DATA(128 bytes). Total 130 bytes. Unsecured
#define BL_VERIFY_PAGE        0x84  // Verify checksum for 1 FLASH page. Format: COM Page# CRC_high CRC_low. Total 4 bytes
#define BL_ERASE_EEPROM       0x87  // Erase 512 bytes of the EEPROM memory. Format: COM
#define BL_WR_EEPROM_L        0x88  // Write one byte to the EEPROM. Format: COM Addr(0...0xFF) DATAbyte
#define BL_WR_EEPROM_H        0x89  // Write one byte to the EEPROM. Format: COM Addr(0x100...0x1FF) DATAbyte
#define BL_VERIFY_EEPROM      0x8A  // Verify checksum for 1 EEPROM page of 16bytes. Format: COM Page# CRC_high CRC_low
#define BL_RUN_USERAPP        0x8C  // Goto user application. Format: COM
#define BL_GET_MCUTYPE        0x8E  // Readout of MCU Type. Format: COM. Return: 0=ATmega164P, 1=ATmega324P, 2=ATmega644P
#define BL_GET_FWNUM	        0x8F	// Readout of firmware revision Format: COM.  Return: 0...255 (rev.- 0.0...25.5)
#define BL_SYNC               0x90  // Synchro sequence. Format: COM 0x01 0x12 0xAA 0x34 0x55 0x59, 0xca Return:"SY"

#define BL_ANS_OK             0xAA  // response code "Ok"
#define BL_ANS_ERR            0xEE  // response code "Error"

#define BL_ANS_SYNC1          'S'   // answer for synchronization packet
#define BL_ANS_SYNC2          'Y'

#define BL_UART_NUM           UART4

#define BL_UART_BAUDRATE     	(57600L)

#define BL_UART_CLOCK_ENABLE  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE)

#define BL_UART_GPIO          GPIOA

#define BL_UART_RX_SOURCE     GPIO_PinSource1
#define BL_UART_TX_SOURCE     GPIO_PinSource0

#define BL_UART_RX_PIN        (1 << BL_UART_RX_SOURCE)
#define BL_UART_TX_PIN        (1 << BL_UART_TX_SOURCE)

/**< Settings for RS485 bootloader */

/**< Settings for CAN bootloader */


//void BOOTLOADER_Task(void *pParameters);
bool BOOTLOADER_Start(void);
void BOOTLOADER_Stop(void);
bool BOOTLOADER_CheckCRC(uint8_t page, uint16_t crc);
bool BOOTLOADER_CheckEEPROM(uint8_t page, uint16_t crc);
bool BOOTLOADER_WriteFlash(uint8_t page, bool secured);
bool BOOTLOADER_WriteEEPROM(uint16_t addr, uint8_t val);

#endif
