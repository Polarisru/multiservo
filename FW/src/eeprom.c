#include "driver_flash.h"
#include "crc16.h"
#include "eeprom.h"
#include "global.h"

uint8_t EEPROM_Page[EEPROM_PAGE_SIZE];

/**< This array is used to match global variables from RAM with their EEPROM representation */
const eeVal_t EEPROM_Values[] =
{
  {0x10, 2, (void*)&EE_CurrOffset,      {.word = 0}},
  {0x12, 4, (void*)&EE_VoltSupplyDiv,   {.fl = 7.67f}}
};

/** \brief Recalculate EEPROM CRC
 *
 * \return New CRC16 value as uint16_t
 *
 */
uint16_t EEPROM_RecalculateCRC(void)
{
  uint16_t crc = CRC16_INIT_VAL;

  crc = CRC16_Calc(crc, &EEPROM_Page[sizeof(uint32_t)], EEPROM_PAGE_SIZE - sizeof(uint32_t));
  /**< Write new CRC value to the start of EEPROM page */
  memcpy(EEPROM_Page, (uint8_t*)&crc, sizeof(uint16_t));

  return crc;
}

/** \brief Calculate EEPROM CRC
 *
 * \return CRC16 value as uint16_t
 *
 */
uint16_t EEPROM_CalculateCRC(void)
{
  uint16_t crc = CRC16_INIT_VAL;

  crc = CRC16_Calc(crc, &EEPROM_Page[sizeof(uint32_t)], EEPROM_PAGE_SIZE - sizeof(uint32_t));

  return crc;
}

/** \brief Get EEPROM CRC value
 *
 * \return CRC16 value as uint16_t
 *
 */
uint16_t EEPROM_GetCRC(void)
{
  return *((uint16_t*)EEPROM_Page);
}

/** \brief Load all variables from EEPROM
 *
 * \return Nothing
 *
 */
void EEPROM_LoadVariables(void)
{
  uint8_t i;

  for (i = 0; i < sizeof(EEPROM_Values) / sizeof(eeVal_t); i++)
  {
    memcpy(EEPROM_Values[i].pVal, &EEPROM_Page[EEPROM_Values[i].address], EEPROM_Values[i].size);
  }
}

/** \brief Rewrite variable with address
 *
 * \param [in] Pointer to variable
 * \return Nothing
 *
 */
void EEPROM_SaveVariable(void *var)
{
  uint8_t i;

  for (i = 0; i < sizeof(EEPROM_Values) / sizeof(eeVal_t); i++)
  {
    if (var == EEPROM_Values[i].pVal)
    {
      memcpy(&EEPROM_Page[EEPROM_Values[i].address], var, EEPROM_Values[i].size);
      EEPROM_RecalculateCRC();
      break;
    }
  }
}

/** \brief Load EEPROM copy to global variables
 *
 * \param [in] ee_copy Number of the copy to load
 * \return Nothing
 *
 */
void EEPROM_LoadCopy(uint8_t ee_copy)
{
  uint32_t addr;

  switch (ee_copy)
  {
    case EE_COPY_1:
      addr = EEPROM_ADDR_COPY_1;
      break;
    case EE_COPY_2:
      addr = EEPROM_ADDR_COPY_2;
      break;
    default:
      return;
  }

  memcpy(EEPROM_Page, (uint8_t*)addr, EEPROM_PAGE_SIZE);
  EEPROM_LoadVariables();
}

/** \brief Save global variables to EEPROM copy
 *
 * \param [in] ee_copy Number of the copy to save to
 * \return Nothing
 *
 */
void EEPROM_SaveCopy(uint8_t ee_copy)
{
  uint32_t addr;
  uint16_t i;
  uint16_t crc;

  switch (ee_copy)
  {
    case EE_COPY_1:
      addr = EEPROM_ADDR_COPY_1;
      break;
    case EE_COPY_2:
      addr = EEPROM_ADDR_COPY_2;
      break;
    default:
      return;
  }
  crc = EEPROM_RecalculateCRC();
  if (crc != *(uint16_t*)addr)
  {
    /**< Save only if CRC is different */
    /**< Erase EEPROM page */
    FLASH_EraseRowEE((uint32_t*)addr);
    /**< Write page content to EEPROM */
    for (i = 0; i < EEPROM_PAGE_SIZE; i += FLASH_PAGE_SIZE)
      FLASH_WriteWordsEE((uint32_t*)(addr + i), (uint32_t*)&EEPROM_Page[i], FLASH_PAGE_SIZE / sizeof(uint32_t));
  }
}

/** \brief Save both EEPROM copies
 *
 * \return Nothing
 *
 */
void EEPROM_SaveBoth(void)
{
  EEPROM_SaveCopy(EE_COPY_1);
  EEPROM_SaveCopy(EE_COPY_2);
}

/** \brief Initialize EEPROM memory, perform self-test
 *
 * \return Nothing
 *
 */
void EEPROM_Init(void)
{
  uint16_t crc;
  uint16_t crc_read;
  bool firstCopy, secondCopy;
  uint8_t i;

  firstCopy = true;
  secondCopy = true;
  EEPROM_LoadCopy(EE_COPY_1);
  crc_read = *(uint16_t*)EEPROM_Page;
  crc = CRC16_INIT_VAL;
  crc = CRC16_Calc(crc, &EEPROM_Page[sizeof(uint32_t)], EEPROM_PAGE_SIZE - sizeof(uint32_t));
  if (crc != crc_read)
  {
    /**< first EEPROM page is corrupted */
    firstCopy = false;
  }
  EEPROM_LoadCopy(EE_COPY_2);
  crc_read = *(uint16_t*)EEPROM_Page;
  crc = CRC16_INIT_VAL;
  crc = CRC16_Calc(crc, &EEPROM_Page[sizeof(uint32_t)], EEPROM_PAGE_SIZE - sizeof(uint32_t));
  if (crc != crc_read)
  {
    /**< second EEPROM page is corrupted */
    secondCopy = false;
  }

  if ((secondCopy == false) && (secondCopy == false))
  {
    /**< Both copies are corrupted Reload default values */
    memset(EEPROM_Page, 0xff, EEPROM_PAGE_SIZE);
    for (i = 0; i < sizeof(EEPROM_Values) / sizeof(eeVal_t); i++)
    {
      memcpy(EEPROM_Values[i].pVal, &EEPROM_Values[i].defVal, EEPROM_Values[i].size);
      memcpy(&EEPROM_Page[EEPROM_Values[i].address], &EEPROM_Values[i].defVal, EEPROM_Values[i].size);
    }
    /**< Save both copies */
    EEPROM_SaveBoth();
  } else
  if (secondCopy == false)
  {
    /**< Second copy is corrupted, reload with first copy */
    EEPROM_LoadCopy(EE_COPY_1);
    EEPROM_SaveCopy(EE_COPY_2);
  } else
  if (firstCopy == false)
  {
    /**< First copy is corrupted, reload with second copy */
    EEPROM_LoadCopy(EE_COPY_2);
    EEPROM_SaveCopy(EE_COPY_1);
  } else
  {
    /**< Both copies are Ok, just load first copy */
    EEPROM_LoadCopy(EE_COPY_1);
  }
}
