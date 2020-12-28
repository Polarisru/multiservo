#include <string.h>
#include "driver_can.h"
#include "hpl_can_config.h"
#include "hpl_can_base.h"
#include "err_codes.h"

#ifdef BOOTLOADER
  #define CAN_GCLK_SRC     GCLK_PCHCTRL_GEN_GCLK0
#else
  #define CAN_GCLK_SRC     GCLK_PCHCTRL_GEN_GCLK0//GCLK_PCHCTRL_GEN_GCLK2
#endif

uint8_t can_rx_fifo[CONF_CAN0_F0DS * CONF_CAN0_RXF0C_F0S];
uint8_t can_tx_fifo[CONF_CAN0_TBDS * CONF_CAN0_TXBC_TFQS];
static struct _can_tx_event_entry can_tx_event_fifo[CONF_CAN0_TXEFC_EFS];
static struct _can_standard_message_filter_element can_rx_std_filter[CONF_CAN0_SIDFC_LSS];
static struct _can_extended_message_filter_element can_rx_ext_filter[CONF_CAN0_XIDFC_LSS];

/** \brief Read a CAN message from CAN bus
 *
 * \param [in] channel Number of the CAN channel
 * \param [in] msg Pointer for CAN message to receive
 * \return Error code
 *
 */
int32_t CAN_ReadMsg(Can *channel, struct can_message *msg)
{
	const uint8_t dlc2len[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 24, 32, 48, 64};
	struct _can_rx_fifo_entry *f = NULL;
	uint32_t get_index;

	if (!((channel->RXF0S.reg & CAN_RXF0S_F0FL_Msk) >> CAN_RXF0S_F0FL_Pos))
		return ERR_NOT_FOUND;

  get_index = (channel->RXF0S.reg & CAN_RXF0S_F0GI_Msk) >> CAN_RXF0S_F0GI_Pos;

  f = (struct _can_rx_fifo_entry *)(can_rx_fifo + get_index * CONF_CAN0_F0DS);
	if (f == NULL)
		return ERR_NO_RESOURCE;

	if (f->R0.bit.XTD == 1)
  {
		msg->fmt = CAN_FMT_EXTID;
		msg->id  = f->R0.bit.ID;
	} else
	{
		msg->fmt = CAN_FMT_STDID;
		/**< A standard identifier is stored into ID[28:18] */
		msg->id = f->R0.bit.ID >> 18;
	}

	if (f->R0.bit.RTR == 1)
		msg->type = CAN_TYPE_REMOTE;

	msg->len = dlc2len[f->R1.bit.DLC];

	memcpy(msg->data, f->data, msg->len);

	channel->RXF0A.bit.F0AI = get_index;

	return ERR_NONE;
}

/** \brief Write a CAN message to bus
 *
 * \param [in] channel Number of the CAN channel
 * \param [in] msg CAN message to write
 * \return Error code
 *
 */
int32_t CAN_WriteMsg(Can *channel, struct can_message *msg)
{
	struct _can_tx_fifo_entry *f = NULL;
  if ((channel->TXFQS.reg & CAN_TXFQS_TFQF) >> CAN_TXFQS_TFQF_Pos)
		return ERR_NO_RESOURCE;
  f = (struct _can_tx_fifo_entry *)(can_tx_fifo + ((channel->TXFQS.reg & CAN_TXFQS_TFQPI_Msk) >> CAN_TXFQS_TFQPI_Pos) * CONF_CAN0_TBDS);
	if (f == NULL)
		return ERR_NO_RESOURCE;

	if (msg->fmt == CAN_FMT_EXTID)
  {
		f->R0.val     = msg->id;
		f->R0.bit.XTD = 1;
	} else
	{
		/**< A standard identifier is stored into ID[28:18] */
		f->R0.val = msg->id << 18;
	}

	if (msg->len <= 8)
  {
		f->R1.bit.DLC = msg->len;
	} else
	if (msg->len <= 12)
	{
		f->R1.bit.DLC = 0x9;
	} else
	if (msg->len <= 16)
	{
		f->R1.bit.DLC = 0xA;
	} else
	if (msg->len <= 20)
	{
		f->R1.bit.DLC = 0xB;
	} else
	if (msg->len <= 24)
	{
		f->R1.bit.DLC = 0xC;
	} else
	if (msg->len <= 32)
	{
		f->R1.bit.DLC = 0xD;
	} else
	if (msg->len <= 48)
	{
		f->R1.bit.DLC = 0xE;
	} else
	if (msg->len <= 64)
	{
		f->R1.bit.DLC = 0xF;
	}

	memcpy(f->data, msg->data, msg->len);

	channel->TXBAR.reg = 1 << ((channel->TXFQS.reg & CAN_TXFQS_TFQPI_Msk) >> CAN_TXFQS_TFQPI_Pos);
	return ERR_NONE;
}

/** \brief Set CAN filter (standard mask)
 *
 * \param [in] index Index of the filter
 * \param [in] fmt Format of the filter (standard or extended)
 * \param [in] filter Pointer to structure with classic filter settings (mask/value)
 * \return Error code
 *
 */
int32_t CAN_SetFilter(uint8_t index, enum can_format fmt, struct can_filter *filter)
{
	struct _can_standard_message_filter_element *sf;
	struct _can_extended_message_filter_element *ef;

	sf = &can_rx_std_filter[index];
	ef = &can_rx_ext_filter[index];

	if (fmt == CAN_FMT_STDID)
  {
		if (filter == NULL)
		{
			sf->S0.val = 0;
			return ERR_NONE;
		}
		sf->S0.val       = filter->mask;
		sf->S0.bit.SFID1 = filter->id;
		sf->S0.bit.SFT   = _CAN_SFT_CLASSIC;
		sf->S0.bit.SFEC  = _CAN_SFEC_STF0M;
	} else if (fmt == CAN_FMT_EXTID)
	{
		if (filter == NULL)
		{
			ef->F0.val = 0;
			return ERR_NONE;
		}
		ef->F0.val      = filter->id;
		ef->F0.bit.EFEC = _CAN_EFEC_STF0M;
		ef->F1.val      = filter->mask;
		ef->F1.bit.EFT  = _CAN_EFT_CLASSIC;
	}

	return ERR_NONE;
}

/** \brief Set CAN filter (range)
 *
 * \param [in] index Index of the filter
 * \param [in] fmt Format of the filter (standard or extended)
 * \param [in] start_id First ID of the range
 * \param [in] stop_id Last ID of the range
 * \return Error code
 *
 */
int32_t CAN_SetRangeFilter(uint8_t index, enum can_format fmt, uint32_t start_id, uint32_t stop_id)
{
	struct _can_standard_message_filter_element *sf;
	struct _can_extended_message_filter_element *ef;

	sf = &can_rx_std_filter[index];
	ef = &can_rx_ext_filter[index];

	if (fmt == CAN_FMT_STDID)
  {
		if (start_id > stop_id)
		{
			sf->S0.val = 0;
			return ERR_NONE;
		}
		sf->S0.bit.SFID1 = start_id;
		sf->S0.bit.SFID2 = stop_id;
		sf->S0.bit.SFT   = _CAN_EFT_RANGE;
		sf->S0.bit.SFEC  = _CAN_SFEC_STF0M;
	} else if (fmt == CAN_FMT_EXTID)
	{
		if (start_id > stop_id)
		{
			ef->F0.val = 0;
			return ERR_NONE;
		}
		ef->F0.val      = start_id;
		ef->F0.bit.EFEC = _CAN_EFEC_STF0M;
		ef->F1.val      = stop_id;
		ef->F1.bit.EFT  = _CAN_EFT_RANGE;
	}

	return ERR_NONE;
}

/** \brief Enable CAN interface
 *
 * \param [in] channel Number of the CAN channel
 * \return Nothing
 *
 */
void CAN_Enable(Can *channel)
{
	channel->CCCR.reg &= ~CAN_CCCR_CCE;
	channel->CCCR.reg &= ~CAN_CCCR_INIT;
	while ((channel->CCCR.reg & CAN_CCCR_INIT) != 0);

  /**< New message received interrupt enable */
  #ifndef BOOTLOADER
  channel->IE.bit.RF0NE = 1;
  #endif
}

/** \brief Disable CAN interface
 *
 * \param [in] channel Number of the CAN channel
 * \return Nothing
 *
 */
void CAN_Disable(Can *channel)
{
  /**< Put CAN to initializing mode */
  channel->CCCR.reg |= CAN_CCCR_INIT;
  while ((channel->CCCR.reg & CAN_CCCR_INIT) == 0);
  channel->CCCR.reg |= CAN_CCCR_CCE;

  /**< Disable Rx interrupt */
  channel->IE.bit.RF0NE = 0;
  /**< Clear Rx flag */
  channel->IR.bit.RF0N = 1;
}

/** \brief Set baudrate
 *
 * \param [in] nominal_baudrate Nominal baudrate
 * \param [in] data_baudrate Data baudrate (CAN FD only)
 * \return bool True if succeed
 *
 */
bool CAN_SetBaudrate(Can *channel, uint32_t nominal_baudrate, uint32_t data_baudrate)
{
  /**< Nominal bit timing and prescaling */
  channel->NBTP.reg = CONF_CAN0_BTP_REG;
  /**< Data bit timing and prescaling */
  channel->DBTP.reg = CONF_CAN0_DBTP_REG;
}

/** \brief Check if CAN bus is stopped and restore it
 *
 * \param [in] channel Number of the CAN channel
 * \return Nothing
 *
 */
void CAN_CheckBus(Can *channel)
{
  uint32_t ir;

  ir = channel->IR.reg;
  if (ir & CAN_IR_BO)
  {
    /**< Bus_Off occurred, CAN should be reinitialized */
    channel->IR.reg = ir;
    CAN_Enable(channel);
  }
}

/** \brief Enable CAN module, no parameters because of complexity
 *
 * \param [in] channel Number of the CAN channel
 * \return Nothing
 *
 */
void CAN_Init(Can *channel)
{
  /**< Configure clock */
  if (channel == CAN0)
  {
    MCLK->AHBMASK.reg |= MCLK_AHBMASK_CAN0;
    GCLK->PCHCTRL[CAN0_GCLK_ID].reg = CAN_GCLK_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  #ifdef CAN1
  if (channel == CAN1)
  {
    MCLK->AHBMASK.reg |= MCLK_AHBMASK_CAN1;
    GCLK->PCHCTRL[CAN1_GCLK_ID].reg = CAN_GCLK_SRC | (1 << GCLK_PCHCTRL_CHEN_Pos);
  } else
  #endif
  {
    /**< Non-available CAN connection */
    return;
  }

  /**< Put CAN to initializing mode */
  channel->CCCR.reg |= CAN_CCCR_INIT;
  while ((channel->CCCR.reg & CAN_CCCR_INIT) == 0);
  channel->CCCR.reg |= CAN_CCCR_CCE;

  /**< Common settings */
  channel->CCCR.reg |= CONF_CAN0_CCCR_REG;
  /**< Message RAM configuration (memory priority access during read/write operations) */
  channel->MRCFG.reg = CONF_CAN0_MRCFG_REG;
  /**< Nominal bit timing and prescaling */
  channel->NBTP.reg = CONF_CAN0_BTP_REG;
  /**< Data bit timing and prescaling */
  channel->DBTP.reg = CONF_CAN0_DBTP_REG;
  /**< Rx FIFO 0 Configuration */
  channel->RXF0C.reg = CONF_CAN0_RXF0C_REG | CAN_RXF0C_F0SA((uint32_t)can_rx_fifo);
  /**< Rx Buffer / FIFO Element Size Configuration */
  channel->RXESC.reg = CONF_CAN0_RXESC_REG;
  /**< Tx Buffer Element Size Configuration */
  channel->TXESC.reg = CONF_CAN0_TXESC_REG;
  /**< Tx Buffer Configuration */
  channel->TXBC.reg = CONF_CAN0_TXBC_REG | CAN_TXBC_TBSA((uint32_t)can_tx_fifo);
  /**< Tx Event FIFO Configuration */
  channel->TXEFC.reg = CONF_CAN0_TXEFC_REG | CAN_TXEFC_EFSA((uint32_t)can_tx_event_fifo);
  /**< Global Filter Configuration */
  channel->GFC.reg = CONF_CAN0_GFC_REG;
  /**< Standard filter configuration */
  channel->SIDFC.reg = CONF_CAN0_SIDFC_REG | CAN_SIDFC_FLSSA((uint32_t)can_rx_std_filter);
  /**< Extended filter configuration */
  channel->XIDFC.reg = CONF_CAN0_XIDFC_REG | CAN_XIDFC_FLESA((uint32_t)can_rx_ext_filter);
  /**< Extended ID AND mask, should have all ones! */
  channel->XIDAM.reg = CONF_CAN0_XIDAM_REG;
}
