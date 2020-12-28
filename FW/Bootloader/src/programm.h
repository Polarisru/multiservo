#ifndef _PROGRAMM_H
#define _PROGRAMM_H

#define SECTORS_NUM       12

#define PACKET_SIZE       8
#define PACKET_OFFS_HEAD  0
#define PACKET_OFFS_CMD   1
#define PACKET_OFFS_PAGE  2
#define PACKET_OFFS_DATA  4
#define PACKET_OFFS_CRC   6

#define PACKET_HEADER     0xA5

#define START_DELAY       5000000

#define CMD_FLASH_PAGE    0x81    // FLASH one page (256b). Format: COM ID ZregH ZregL 0x00 0x00 CRCH CRCL
#define CMD_CHECK_FLASH   0x83    // Verify checksum for 1 FLASH page. Format: COM ID page# 0x00 CRC_pageH CRC_pageL CRCH CRCL
#define CMD_FILL_BUFF   	0x86    // Write the whole page to tmp buffer. The same format as Fill_Tmp_Buf_long, but data bytes are unprotected. No response
#define CMD_RUN_APP       0xA0    // Goto user application. Format: COM ID 0x00 0x00 0x00 0x00 CRCH CRCL
#define CMD_SYNC          0x90    // synchronization command

#define ANSW_ERROR        'E'
#define ANSW_OK			      'A'

#define ANSW_SYNC1        'S'
#define ANSW_SYNC2        'Y'

#define DEVICE_ID      		  0       // LS1

#define RAM_START_ADDR      0x20002000
#define BLOCK_SIZE			    256

#define FLASH_KEY1          ((uint32_t)0x45670123)
#define FLASH_KEY2          ((uint32_t)0xCDEF89AB)

#define CMD_ERROR     0
#define CMD_SUCCESS   1

#define RAM_KEY_ADDR        (0x20002000)
#define BOOTLOADER_KEY1     (0x12345678)
#define BOOTLOADER_KEY2     (0x43211234)
#define BOOTLOADER_SIGN     (0x11223344)

#define PAGE_SIZE             256

#define BOOT_SIZE             4096
#define FLASH_APP_ADDRESS     BOOT_SIZE

#endif
