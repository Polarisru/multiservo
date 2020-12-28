#ifndef EEPROM_H
#define EEPROM_H

#include "defines.h"

#define EEPROM_ADDR_COPY_1      0x00400000
#define EEPROM_ADDR_COPY_2      0x00400100

#define EEPROM_ADDR_STRINGS     0x00400200

#define EEPROM_ADDR_COUNTERS    0x00400300

#define EEPROM_ESN_ADDRESS      0x00
#define EEPROM_PROD_ADDRESS     0x30
#define EEPROM_FWREV_ADDRESS    0x60
#define EEPROM_HWREV_ADDRESS    0x90

#define EEPROM_POWERUPS_ADDR    0x20
#define EEPROM_STALLS_ADDR      0x24

#define EEPROM_PAGE_SIZE        0x100

#define EEPROM_WRITE_TIME_MS    3

#define EEPROM_LEN_STRING       32

enum
{
  EE_COPY_1,
  EE_COPY_2
};

typedef struct
{
  uint8_t address;  /**< Address in FRAM */
  uint8_t size;     /**< Size in bytes */
  void*   pVal;     /**< Pointer to variable in RAM */
  union {
    uint8_t  byte;
    uint16_t word;
    uint32_t dword;
    float    fl;
  } defVal;
} eeVal_t;

uint8_t EEPROM_Page[EEPROM_PAGE_SIZE];
uint8_t EEPROM_Strings[EEPROM_PAGE_SIZE];

uint32_t EEPROM_GetCounter(uint8_t num);
void EEPROM_IncCounter(uint8_t num);
void EEPROM_ResetCounter(uint8_t num);
void EEPROM_ResetStalls(void);
void EEPROM_IncStalls(void);
uint32_t EEPROM_GetPowerUps(void);
void EEPROM_IncPowerUps(void);
uint16_t EEPROM_RecalculateCRC(void);
uint16_t EEPROM_CalculateCRC(void);
uint16_t EEPROM_GetCRC(void);
void EEPROM_SaveVariable(void *var);
void EEPROM_LoadVariables(void);
void EEPROM_SaveBoth(void);
void EEPROM_SaveStrings(void);
void EEPROM_SaveCounters(void);
void EEPROM_Init(void);

#endif

