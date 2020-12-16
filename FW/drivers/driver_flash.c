#include "driver_flash.h"

/** \brief Wait till Flash operation is completed
 *
 * \return Nothing
 *
 */
void FLASH_WaitReady(void)
{
  while (NVMCTRL->INTFLAG.bit.READY == 0);
}

/** \brief Erase one row at destination address
 *
 * \param [in] dst Destination address
 * \return Nothing
 *
 */
void FLASH_EraseRow(uint32_t *dst)
{
  FLASH_WaitReady();
  NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

  // Execute "ER" Erase Row
  NVMCTRL->ADDR.reg = (uint32_t)dst / 2;
  NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_ER;
  FLASH_WaitReady();
}

/** \brief Erase one EEPROM row
 *
 * \param [in] dst Destination address at EEPROM
 * \return Nothing
 *
 */
void FLASH_EraseRowEE(uint32_t *dst)
{
  FLASH_WaitReady();
  NVMCTRL->STATUS.reg = NVMCTRL_STATUS_MASK;

  // Execute "ER" Erase Row
  NVMCTRL->ADDR.reg = (uint32_t)dst / 2;
  NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEER;
  FLASH_WaitReady();
}

/** \brief Erase full flash content starting with address
 *
 * \param [in] start_address Starting address for erase operation
 * \return Nothing
 *
 */
void FLASH_EraseFull(uint32_t *start_address)
{
  // Note: the flash memory is erased in ROWS, that is in block of 4 pages.
  // Even if the starting address is the last byte of a ROW the entire ROW
  // is erased anyway.
  uint32_t dst_addr = (uint32_t) start_address; // starting address

  while (dst_addr < FLASH_SIZE)
  {
    FLASH_EraseRow((void *)dst_addr);
    dst_addr += FLASH_ROW_SIZE;
  }
}

/** \brief Write memory buffer to flash
 *
 * \param [in] dst Destination address (Flash)
 * \param [in] src Memory buffer
 * \param [in] n_words Number of dwords (4 bytes) to write
 * \return Nothing
 *
 */
void FLASH_WriteWords(uint32_t *dst, uint32_t *src, uint32_t n_words)
{
  // Set automatic page write
  NVMCTRL->CTRLB.bit.MANW = 0;

  while (n_words > 0)
  {
    uint32_t len = (FLASH_PAGE_SIZE >> 2) < n_words ? (FLASH_PAGE_SIZE >> 2) : n_words;
    n_words -= len;

    // Execute "PBC" Page Buffer Clear
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
    FLASH_WaitReady();
    while (len--)
      *dst++ = *src++;

    // Execute "WP" Write Page
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_WP;
    FLASH_WaitReady();
  }
}

/** \brief Write memory buffer to EEPROM
 *
 * \param [in] dst Destination address (Flash)
 * \param [in] src Memory buffer
 * \param [in] n_words Number of dwords (4 bytes) to write
 * \return Nothing
 *
 */
void FLASH_WriteWordsEE(uint32_t *dst, uint32_t *src, uint32_t n_words)
{
  // Set automatic page write
  NVMCTRL->CTRLB.bit.MANW = 0;

  while (n_words > 0)
  {
    uint32_t len = (FLASH_PAGE_SIZE >> 2) < n_words ? (FLASH_PAGE_SIZE >> 2) : n_words;
    n_words -= len;

    // Execute "PBC" Page Buffer Clear
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_PBC;
    FLASH_WaitReady();
    while (len--)
      *dst++ = *src++;

    // Execute "WP" Write Page
    NVMCTRL->CTRLA.reg = NVMCTRL_CTRLA_CMDEX_KEY | NVMCTRL_CTRLA_CMD_RWWEEWP;
    FLASH_WaitReady();
  }
}

/*int32_t dst = 0x00009000;
uint8_t wr_buff[256];
for (value = 0; value < 256; value++)
  wr_buff[value] = value;
FLASH_EraseRow((uint32_t*)dst);
FLASH_WriteWords((uint32_t*)dst, (uint32_t*)wr_buff, 256 >> 2);*/
