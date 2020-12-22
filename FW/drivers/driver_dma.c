#include "driver_dma.h"

#define DMA_CRC_TIMER             TC1
#define DMA_CRC_TIMER_MCLK        MCLK_APBCMASK_TC1
#define DMA_CRC_TIMER_PRESCALER   TC_CTRLA_PRESCALER_DIV64_Val
#define DMA_CRC_TIMER_GCLK_FREQ   48000000UL
#define DMA_CRC_TIMER_GCLK_ID     TC1_GCLK_ID
#define DMA_CRC_TIMER_FREQ        (DMA_CRC_TIMER_GCLK_FREQ / 64)
#define DMA_CRC_TIMER_100US_TICK  (uint8_t)(DMA_CRC_TIMER_FREQ / 100)

volatile bool DMA_TransferComplete[DMA_CHANNELS_NUM];

static DmacDescriptor dma_descr[DMA_CHANNELS_NUM]  __attribute__((aligned(16))) SECTION_DMAC_DESCRIPTOR;
static DmacDescriptor writeback[DMA_CHANNELS_NUM] __attribute__((aligned(16))) SECTION_DMAC_DESCRIPTOR;
static DmacDescriptor descriptor  __attribute__((aligned(16))) SECTION_DMAC_DESCRIPTOR;

/**< Interrupt for DMA transfer completion */
void DMAC_Handler(void)
{
  uint8_t channel;
  uint8_t old_channel;
  uint32_t res;

  old_channel = DMAC->CHID.reg;
  res = DMAC->INTSTATUS.reg;
  /**< Get channel number */
  //channel =  DMAC->INTPEND.reg & DMAC_INTPEND_ID_Msk;
  //if (channel < DMA_CHANNELS_NUM)
  for (channel = 0; channel < DMA_CHANNELS_NUM; channel++)
  {
    if (res & (1 << channel))
    {
      /**< Set transfer complete flag */
      DMA_TransferComplete[channel] = true;
      /**< Select channel to work with */
      DMAC->CHID.reg = DMAC_CHID_ID(channel);
      /**< Clear all flags for DMA channel */
      DMAC->CHINTFLAG.reg = (DMAC_CHINTENCLR_TCMPL | DMAC_CHINTENCLR_TERR | DMAC_CHINTENCLR_SUSP);
      /**< Disable DMA transfer */
      DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
    }
  }
  DMAC->CHID.reg = old_channel;
}

/** \brief Initialize DMA module
 *
 * \return Nothing
 *
 */
void DMA_Init(void)
{
  uint8_t i;

  MCLK->AHBMASK.reg |= MCLK_AHBMASK_DMAC;

	DMAC->CTRL.reg &= ~DMAC_CTRL_DMAENABLE;
	DMAC->CTRL.reg = DMAC_CTRL_SWRST;

	DMAC->BASEADDR.reg = (uint32_t)dma_descr;
	DMAC->WRBADDR.reg = (uint32_t)writeback;

  DMAC->CTRL.reg = DMAC_CTRL_DMAENABLE | DMAC_CTRL_LVLEN(0xf);

  for (i = 0; i < DMA_CHANNELS_NUM; i++)
	  DMA_TransferComplete[i] = true;

	NVIC_EnableIRQ(DMAC_IRQn);
	NVIC_SetPriority(DMAC_IRQn, 1);
}

/** \brief Setup DMA transfer for selected channel
 *
 * \param [in] channel Number of the channel to act
 * \param [in] settings Pointer to TDmaSettings structure with DMA settings
 * \return Nothing
 *
 */
void DMA_SetupChannel(uint8_t channel, TDmaSettings *settings)
{
  uint16_t btctrlVal;

  if (channel >= DMA_CHANNELS_NUM)
    return;

  /**< Setup channel */
  DMAC->CHID.reg = DMAC_CHID_ID(channel);
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_SWRST;
  DMAC->SWTRIGCTRL.reg &= (uint32_t)(~(1 << channel));

	if (settings->trig_src == 0)
    DMAC->CHCTRLB.reg = DMAC_CHCTRLB_LVL(settings->priority) | DMAC_CHCTRLB_TRIGACT_BLOCK;
  else
    DMAC->CHCTRLB.reg = DMAC_CHCTRLB_LVL(settings->priority) | DMAC_CHCTRLB_TRIGSRC(settings->trig_src) | DMAC_CHCTRLB_TRIGACT_BEAT;
  /**< Enable interrupts */
  DMAC->CHINTENSET.reg = DMAC_CHINTENSET_MASK;
  /**< Prepare descriptor block */
  descriptor.DESCADDR.reg = 0;          // only one block to transfer, so 0
  descriptor.BTCNT.reg = settings->len;
  btctrlVal = DMAC_BTCTRL_VALID | DMAC_BTCTRL_BEATSIZE(settings->beat_size);
  descriptor.DSTADDR.reg = (uint32_t)settings->dst_addr;
  if (settings->dst_inc == true)
  {
    descriptor.DSTADDR.reg += settings->len * (1 << settings->beat_size);
    btctrlVal |= DMAC_BTCTRL_DSTINC;
  }
  descriptor.SRCADDR.reg = (uint32_t)settings->src_addr;
  if (settings->src_inc == true)
  {
    descriptor.SRCADDR.reg += settings->len * (1 << settings->beat_size);
    btctrlVal |= DMAC_BTCTRL_SRCINC;
  }
  descriptor.BTCTRL.reg = btctrlVal;

  memcpy(&dma_descr[channel], &descriptor, sizeof(DmacDescriptor));
	/**< Start DMA transfer */
//	DMA_TransferComplete[channel] = false;
//	/**< Probably software trigger should be used to initiate transfer */
//	if (settings->trig_src == 0)
//    DMAC->SWTRIGCTRL.reg |= (uint32_t)(1 << channel);
//	DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

/** \brief Start DMA transfer, channel should be already initialized
 *
 * \param [in] channel Number of the channel to act
 * \return Nothing
 *
 */
void DMA_StartChannel(uint8_t channel)
{
  if (channel >= DMA_CHANNELS_NUM)
    return;

  /**< Setup channel */
  DMAC->CHID.reg = DMAC_CHID_ID(channel);
  if (DMA_TransferComplete[channel] == false)
    DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;

	/**< Start DMA transfer */
	DMA_TransferComplete[channel] = false;
	DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
}

void DMA_StopChannel(uint8_t channel)
{
  if (channel >= DMA_CHANNELS_NUM)
    return;

  DMAC->CHID.reg = DMAC_CHID_ID(channel);
  DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
}

/** \brief Check if DMA transmission is ready
 *
 * \param [in] channel Number of the channel to act
 * \return True if transmission is ready
 *
 */
bool DMA_IsReady(uint8_t channel)
{
  if (channel >= DMA_CHANNELS_NUM)
    return true;

  return DMA_TransferComplete[channel];
}

/** \brief Initialize CRC module
 *
 * \param
 * \param
 * \return Nothing
 *
 */
void DMA_InitCRC(void)
{
  /**< Power on timer */
  MCLK->APBCMASK.reg |= DMA_CRC_TIMER_MCLK;
  GCLK->PCHCTRL[DMA_CRC_TIMER_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  /**< Setup timer */
  DMA_CRC_TIMER->COUNT8.CTRLA.reg = TC_CTRLA_SWRST;
  while (DMA_CRC_TIMER->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);
  DMA_CRC_TIMER->COUNT8.CTRLA.reg |= (DMA_CRC_TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | TC_CTRLA_MODE_COUNT8;
  DMA_CRC_TIMER->COUNT8.CTRLBSET.bit.DIR = 0;
  DMA_CRC_TIMER->COUNT8.PER.reg = DMA_CRC_TIMER_100US_TICK;
  DMA_CRC_TIMER->COUNT8.COUNT.reg = 0;
  DMA_CRC_TIMER->COUNT8.CTRLA.reg |= (TC_CTRLA_ENABLE);

  DMAC->CTRL.reg &= ~DMAC_CTRL_CRCENABLE;
  /**< Configure the CRC engine */
  DMAC_CRCCTRL_Type crcctrl =
  {
    .bit.CRCSRC = DMAC_CRCCTRL_CRCSRC_IO_Val, /* I/O interface */
    .bit.CRCPOLY = DMAC_CRCCTRL_CRCPOLY_CRC16_Val, /* CRC-16 (CRC-CCITT)  */
    .bit.CRCBEATSIZE = DMAC_CRCCTRL_CRCBEATSIZE_BYTE_Val, /* Byte bus access */
  };
  DMAC->CRCCTRL.reg = crcctrl.reg;
}

/** \brief Start CRC conversion
 *
 * \param [in] crc_init CRC initial value
 * \return Nothing
 *
 */
void DMA_StartCRC(uint32_t crc_init)
{
  /**< Clear the busy bit */
  DMAC->CRCSTATUS.bit.CRCBUSY = 1;
  DMAC->CTRL.bit.CRCENABLE = 0;
  /**< Initialize start CRC value for the CRC16 */
  DMAC->CRCCHKSUM.reg = crc_init;
  /**< Enable the CRC engine */
  DMAC->CTRL.bit.CRCENABLE = 1;
}

/** \brief Get CRC result from DMA module
 *
 * \return CRC as uint16_t
 *
 */
uint16_t DMA_GetCRC(void)
{
  return (uint16_t)DMAC->CRCCHKSUM.reg;
}
