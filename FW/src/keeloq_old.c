#include "driver_gpio.h"
#include "driver_dma.h"
#include "keeloq.h"

enum {
  KEELOQ_MODE_SEND,
  KEELOQ_MODE_REQUEST
};

#define DMA_CHANNEL_KEELOQ      0
#define DMA_CHANNEL_IN_KEELOQ   1

#define KEELOQ_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV64_Val
#define KEELOQ_TIMER_VALUE      (CONF_GCLK_TC0_FREQUENCY / 1000000UL * KEELOQ_BIT_LEN / 64)

#define KEELOQ_PORT             GPIO_PORTA
#define KEELOQ_PIN              10

#define KEELOQ_IN_PORT          GPIO_PORTA
#define KEELOQ_IN_PIN           11

#define KEELOQ_BIT_LEAVE        0
#define KEELOQ_BIT_TOGGLE       (1 << KEELOQ_PIN)

#define KEELOQ_RETRIES          3         // number of retries

#define KEELOQ_IN_TIMER_MAX_VALUE   1000

#define KEELOQ_IN_TIMER       TCC0

uint32_t KEELOQ_Buff[128];
uint32_t KEELOQ_InBuff[128];
uint8_t KEELOQ_Pos, KEELOQ_Last;
uint8_t KEELOQ_Mode;
static bool KEELOQ_RxReady;

/**< Interrupt handler for external interrupts */
void EIC_Handler(void)
{
  EIC->INTFLAG.reg = (1 << KEELOQ_IN_PIN);
}


//void TC2_Handler(void)
//{
//  TC2->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
//  PORT->Group[KEELOQ_PORT].OUTTGL.reg = KEELOQ_Buff[KEELOQ_Pos++];
//  if (KEELOQ_Pos >= KEELOQ_Last)
//    TC2->COUNT8.CTRLA.bit.ENABLE = 0;
//}

bool KEELOQ_Transfer(uint8_t *request, uint8_t *reply)
{
	uint8_t i, j;
	uint8_t pos = 0;
	uint8_t byte;
	uint8_t crc = 0;

  if (reply == NULL)
    KEELOQ_Mode = KEELOQ_MODE_SEND;
  else
    KEELOQ_Mode = KEELOQ_MODE_REQUEST;

  /**< Configure pin as output */
  //CAPTURE_Disable();
  //CAPTURE_DisablePin();

  /**< Initial low-level for better preamble recognition */
  GPIO_ClearPin(KEELOQ_PORT, KEELOQ_PIN);
  vTaskDelay(10);

	/**< Set preamble */
	for (i = 0; i < KEELOQ_PREAMBLE_LEN; i++)
	{
	  KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
	  KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
	}

	/**< Set header (just set low and delay for 2 ms) */
	for (i = 0; i < KEELOQ_HEADER_LEN / KEELOQ_BIT_LEN; i++)
    KEELOQ_Buff[pos++] = KEELOQ_BIT_LEAVE;

	/**< Set data bytes */
	for (i = 0; i < KEELOQ_PACKET_LEN; i++)
	{
		if (i < (KEELOQ_PACKET_LEN - 1))
		{
			byte = request[i];
			crc ^= byte;
		} else
		{
			byte = crc;
		}

		for (j = 0; j < 8; j++)
		{
		  KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
			if (byte & 0x01)
      {
			  KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
			  KEELOQ_Buff[pos++] = KEELOQ_BIT_LEAVE;
      } else
      {
        KEELOQ_Buff[pos++] = KEELOQ_BIT_LEAVE;
        KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
      }
			byte >>= 1;
		}
	}

	/**< Set stop bit */
	KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;
	KEELOQ_Buff[pos++] = KEELOQ_BIT_LEAVE;
	KEELOQ_Buff[pos++] = KEELOQ_BIT_TOGGLE;

  KEELOQ_RxReady = false;

//  DMA_StartChannel(DMA_CHANNEL_IN_KEELOQ);
//  KEELOQ_IN_TIMER->CTRLA.reg |= (TCC_CTRLA_ENABLE);
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & (TCC_SYNCBUSY_SWRST | TCC_SYNCBUSY_ENABLE));

  /**< Start DMA transfer */
  //TC2->COUNT8.COUNT.reg = TC2->COUNT8.PER.reg;
  DMA_StartChannel(DMA_CHANNEL_KEELOQ);


  vTaskDelay(30);
  reply[0] = request[0] & 0x7f;
  reply[1] = request[1];

  /**< Stop timer */
//  KEELOQ_IN_TIMER->CTRLA.reg &= (~TCC_CTRLA_ENABLE);
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & (TCC_SYNCBUSY_SWRST | TCC_SYNCBUSY_ENABLE));
//  DMA_StopChannel(DMA_CHANNEL_IN_KEELOQ);

  return true;
}

/** \brief Write one byte to KeeLoq device at address
 *
 * \param [in] addr Address to write to
 * \param [in] value Value to write
 * \return true if succeed
 *
 */
bool KEELOQ_Write(uint8_t addr, uint8_t value)
{
  uint8_t buff[2] = {0};
  uint8_t rec[2] = {0};
  uint8_t i = 0;
  bool res;

  if (addr & 0x80)
    return true;

  buff[0] = addr | 0x80;
  buff[1] = value;
  while (i < KEELOQ_RETRIES)
  {
    i++;
    res = KEELOQ_Transfer(buff, rec);
    /**< Enable PWM */
    //OUTPUTS_Switch(OUTPUTS_PWM_EN, OUTPUTS_SWITCH_ON);
    if (res == false)
      continue;
    if ((rec[0] != addr) || (rec[1] != value))
      continue;
    return true;
  }

  return false;
}

/** \brief Read one byte from KeeLoq device at address
 *
 * \param [in] addr Address to read from
 * \param [out] value Pointer to read value
 * \return true if succeed
 *
 */
bool KEELOQ_Read(uint8_t addr, uint8_t *value)
{
  uint8_t buff[2] = {0};
  uint8_t rec[2] = {0};
  uint8_t i = 0;
  bool res;

  buff[0] = addr;
  while (i < KEELOQ_RETRIES)
  {
    i++;
    res = KEELOQ_Transfer(buff, rec);
    /**< Enable PWM */
    //OUTPUTS_Switch(OUTPUTS_PWM_EN, OUTPUTS_SWITCH_ON);
    if (res == false)
      continue;
    if (buff[0] != rec[0])
      continue;
    *value = rec[1];
    return true;
  }

  return false;
}

/** \brief Initialize KeeLoq module (event timer and DMA transfer)
 *
 * \param
 * \param
 * \return
 *
 */
void KEELOQ_Init(void)
{
  TDmaSettings DmaSett;

  /**< Setup timer as events source */
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC2;
  GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  TC2->COUNT8.CTRLA.bit.SWRST = 1;
  while (TC2->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);
  TC2->COUNT8.CTRLA.reg = (KEELOQ_TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | TC_CTRLA_MODE_COUNT8;
  TC2->COUNT8.CTRLBSET.reg = 0;
  TC2->COUNT8.EVCTRL.reg = TC_EVCTRL_OVFEO;
  TC2->COUNT8.PER.reg = KEELOQ_TIMER_VALUE;
  TC2->COUNT8.CTRLA.bit.ENABLE = 1;
  while (TC2->COUNT8.SYNCBUSY.reg & (TC_SYNCBUSY_SWRST | TC_SYNCBUSY_ENABLE));

  DMA_Init();

  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
  DmaSett.trig_src = TC2_DMAC_ID_OVF;
  DmaSett.dst_addr = (void*)&PORT->Group[KEELOQ_PORT].OUTTGL.reg;
  DmaSett.src_addr = (void*)KEELOQ_Buff;
  DmaSett.src_inc = true;
  DmaSett.dst_inc = false;
  DmaSett.len = 126;
  DmaSett.priority = 0;
  DMA_SetupChannel(DMA_CHANNEL_KEELOQ, &DmaSett);

//  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
//  DmaSett.trig_src = TCC0_DMAC_ID_MC_0;
//  DmaSett.src_addr = (void*)&TCC0->CC[0].reg;
//  DmaSett.dst_addr = (void*)KEELOQ_InBuff;
//  DmaSett.src_inc = false;
//  DmaSett.dst_inc = true;
//  DmaSett.len = 128;
//  DmaSett.priority = 0;
//  DMA_SetupChannel(DMA_CHANNEL_IN_KEELOQ, &DmaSett);

  GPIO_ClearPin(KEELOQ_PORT, KEELOQ_PIN);
  GPIO_SetDir(KEELOQ_PORT, KEELOQ_PIN, true);
  GPIO_SetFunction(KEELOQ_PORT, KEELOQ_PIN, GPIO_PIN_FUNC_OFF);

//  /**< Setup power and clock for pin interrupt as event */
//  GCLK->PCHCTRL[EIC_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
//  MCLK->APBAMASK.reg |= MCLK_APBAMASK_EIC;
//
//  /**< Set pin event generation */
//  if (EIC->CTRLA.bit.ENABLE == 0)
//  {
//    EIC->CTRLA.reg = EIC_CTRLA_SWRST;
//    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_SWRST);
//  } else
//  {
//    EIC->CTRLA.bit.ENABLE = 0;
//    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);
//  }
//  EIC->NMICTRL.reg |= (0 << EIC_NMICTRL_NMIFILTEN_Pos) | EIC_NMICTRL_NMISENSE(EIC_NMICTRL_NMISENSE_NONE_Val) | EIC_ASYNCH_ASYNCH(0);
//  //EIC->INTENSET.reg |= (1 << KEELOQ_IN_PIN);
//  EIC->EVCTRL.reg |= (1 << KEELOQ_IN_PIN);
//  EIC->ASYNCH.reg |= (1 << KEELOQ_IN_PIN);
//  EIC->DEBOUNCEN.reg |= (1 << KEELOQ_IN_PIN);
//  EIC->DPRESCALER.reg |= 0;
//  /**< Setup Ext.Pin 6 (CONFIG0) */
//  EIC->CONFIG[0].reg |= 0;
//  EIC->CONFIG[1].reg |= (0 << EIC_CONFIG_FILTEN2_Pos) | EIC_CONFIG_SENSE2(EIC_NMICTRL_NMISENSE_HIGH);
//  EIC->CTRLA.reg |= EIC_CTRLA_ENABLE;
//  while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);

  /**< Initialize interrupts */
//  NVIC_DisableIRQ(EIC_IRQn);
//  NVIC_ClearPendingIRQ(EIC_IRQn);
//  NVIC_EnableIRQ(EIC_IRQn);


//  /**< Setup timer for input capture */
//  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TCC0;
//  GCLK->PCHCTRL[TCC0_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
//
//  /**< Reset TCC module */
//  KEELOQ_IN_TIMER->CTRLA.reg = TCC_CTRLA_SWRST;
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST);
//  KEELOQ_IN_TIMER->CTRLBCLR.reg = TCC_CTRLBCLR_DIR;     /* count up */
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_CTRLB);
//
//  /**< configure the TCC device */
//  KEELOQ_IN_TIMER->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV2_Val) | TCC_CTRLA_CPTEN0 | TCC_CTRLA_CPTEN1);
//  KEELOQ_IN_TIMER->EVCTRL.reg = (TCC_EVCTRL_MCEI0 | TCC_EVCTRL_MCEI1 | TCC_EVCTRL_TCEI1 | TCC_EVCTRL_EVACT1_PPW);
//
//
//  /**< Setup event system */
//  GCLK->PCHCTRL[EVSYS_GCLK_ID_0].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
//  MCLK->APBCMASK.reg |= MCLK_APBCMASK_EVSYS;
//  /**< Event channel 0 setup */
//  EVSYS->CHANNEL[0].reg = (1 << EVSYS_CHANNEL_ONDEMAND_Pos) | (0 << EVSYS_CHANNEL_RUNSTDBY_Pos)
//                       | EVSYS_CHANNEL_EDGSEL(EVSYS_CHANNEL_EDGSEL_BOTH_EDGES_Val) | EVSYS_CHANNEL_PATH(EVSYS_CHANNEL_PATH_ASYNCHRONOUS_Val)
//                       | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_EIC_EXTINT_10);
//  /* <23=>TCC0_EV_1 as event destination, <0x1=>Channel 0 */
//  EVSYS->USER[EVSYS_ID_USER_TCC0_EV_1].reg = 1;

//  GPIO_ClearPin(KEELOQ_IN_PORT, KEELOQ_IN_PIN);
//  GPIO_SetDir(KEELOQ_IN_PORT, KEELOQ_IN_PIN, false);
//  GPIO_SetPullMode(KEELOQ_IN_PORT, KEELOQ_IN_PIN, GPIO_PULLMODE_UP);
//  GPIO_SetFunction(KEELOQ_IN_PORT, KEELOQ_IN_PIN, MUX_PA10A_EIC_EXTINT10);
}


