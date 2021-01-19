#include "keeloq.h"
#include "outputs.h"
#include "pwmout.h"

enum {
  KEELOQ_MODE_SEND,
  KEELOQ_MODE_REQUEST
};

#define KEELOQ_DMA_TIMER        TC2
#define KEELOQ_DMA_TIMER_GCLK   TC2_GCLK_ID
#define KEELOQ_DMA_TIMER_MCLK   MCLK_APBCMASK_TC2
#define KEELOQ_DMA_OUT_ID       TC2_DMAC_ID_OVF
#define KEELOQ_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV64_Val
#define KEELOQ_TIMER_VALUE      (CONF_GCLK_TC0_FREQUENCY / 1000000UL * KEELOQ_BIT_LEN / 64)

#define KEELOQ_PORT             GPIO_PORTA
#define KEELOQ_PIN              0

#define KEELOQ_IN_PORT          GPIO_PORTA
#define KEELOQ_IN_PIN           1
#define KEELOQ_IN_PINMUX        MUX_PA01A_EIC_EXTINT1

#define KEELOQ_BIT_LEAVE        0
#define KEELOQ_BIT_TOGGLE       (1 << KEELOQ_PIN)

#define KEELOQ_RETRIES          3         // number of retries

#define KEELOQ_IN_TIMER_MAX_VALUE   1000

#define KEELOQ_IN_TIMER         TCC2
#define KEELOQ_IN_TIMER_MCLK    MCLK_APBCMASK_TCC2
#define KEELOQ_IN_TIMER_GCLK    TCC2_GCLK_ID
#define KEELOQ_IN_DMA_ID        TCC2_DMAC_ID_MC_1
#define KEELOQ_EVSYS_TIMER_ID   EVSYS_ID_USER_TCC2_EV_1

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

void KEELOQ_SetPinOutput(void)
{
  GPIO_ClearPin(KEELOQ_PORT, KEELOQ_PIN);
  GPIO_SetDir(KEELOQ_PORT, KEELOQ_PIN, true);
  GPIO_SetFunction(KEELOQ_PORT, KEELOQ_PIN, GPIO_PIN_FUNC_OFF);
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_ON);
}

void KEELOQ_SetPinInput(void)
{
  GPIO_ClearPin(KEELOQ_IN_PORT, KEELOQ_IN_PIN);
  GPIO_SetDir(KEELOQ_IN_PORT, KEELOQ_IN_PIN, false);
  GPIO_SetPullMode(KEELOQ_IN_PORT, KEELOQ_IN_PIN, GPIO_PULLMODE_UP);
  GPIO_SetFunction(KEELOQ_IN_PORT, KEELOQ_IN_PIN, KEELOQ_IN_PINMUX);
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_OFF);
  /**< Reset TCC module */
  KEELOQ_IN_TIMER->CTRLA.reg = TCC_CTRLA_SWRST;
  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST);
  KEELOQ_IN_TIMER->CTRLBCLR.reg = TCC_CTRLBCLR_DIR;     /* count up */
  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_CTRLB);
  /**< configure the TCC device */
  KEELOQ_IN_TIMER->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV2_Val) | TCC_CTRLA_CPTEN0 | TCC_CTRLA_CPTEN1);
  KEELOQ_IN_TIMER->EVCTRL.reg = (TCC_EVCTRL_MCEI0 | TCC_EVCTRL_MCEI1 | TCC_EVCTRL_TCEI1 | TCC_EVCTRL_EVACT1_PPW);
  KEELOQ_IN_TIMER->CTRLA.reg |= (TCC_CTRLA_ENABLE);
  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & (TCC_SYNCBUSY_SWRST | TCC_SYNCBUSY_ENABLE));
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
	uint32_t ticks;
	uint8_t buff[KEELOQ_PACKET_LEN] = {0};

  if (reply == NULL)
    KEELOQ_Mode = KEELOQ_MODE_SEND;
  else
    KEELOQ_Mode = KEELOQ_MODE_REQUEST;

  /**< Configure pin as output */
  KEELOQ_SetPinOutput();

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

  /**< Start DMA transfer */
  KEELOQ_DMA_TIMER->COUNT8.COUNT.reg = KEELOQ_DMA_TIMER->COUNT8.PER.reg;
  DMA_StartChannel(DMA_CHANNEL_KEELOQ);

  /**< Wait till transfer is ready */
  ticks = xTaskGetTickCount() + 30;
  while (DMA_IsReady(DMA_CHANNEL_KEELOQ) != true)
  {
    if (xTaskGetTickCount() > ticks)
      return false;
  }

  if (KEELOQ_Mode == KEELOQ_MODE_REQUEST)
  {
    /**< Configure pin as input */
    KEELOQ_SetPinInput();
    /**< Start DMA receiving */
    DMA_StartChannel(DMA_CHANNEL_IN_KEELOQ);

    vTaskDelay(30);

    /**< Stop timer */
    KEELOQ_IN_TIMER->CTRLA.reg &= (~TCC_CTRLA_ENABLE);
    while (KEELOQ_IN_TIMER->SYNCBUSY.reg & (TCC_SYNCBUSY_SWRST | TCC_SYNCBUSY_ENABLE));
    DMA_StopChannel(DMA_CHANNEL_IN_KEELOQ);

    /**< Process data bits */
    pos = 20;
    for (i = 0; i < KEELOQ_PACKET_LEN; i++)
    {
      for (j = 0; j < 8; j++)
      {
        buff[i] >>= 1;
        if ((KEELOQ_InBuff[pos] <= KEELOQ_BIT1_MAXLEN) && (KEELOQ_InBuff[pos] >= KEELOQ_BIT1_MINLEN))
          buff[i] |= 0x80;
        else if ((KEELOQ_InBuff[pos] > KEELOQ_BIT0_MAXLEN) || (KEELOQ_InBuff[pos] < KEELOQ_BIT0_MINLEN))
          return false;
        pos++;
      }
    }

    /**< Check checksum (simple XOR-algorithm) */
    if ((buff[0] ^ buff[1] ^ buff[2]) != 0)
      return false;

    /**< Copy to output buffer without checksum byte */
    memcpy(reply, buff, KEELOQ_PACKET_LEN - 1);
  }

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
    PWMOUT_Configuration();
    PWMOUT_Enable();
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
  bool res = false;

  buff[0] = addr;
  while (i < KEELOQ_RETRIES)
  {
    i++;
    res = KEELOQ_Transfer(buff, rec);
    if (res == false)
      continue;
    if (buff[0] != rec[0])
      continue;
    *value = rec[1];
    break;
  }

  if (i < KEELOQ_RETRIES)
    res = true;
  else
    res = false;

  /**< Enable PWM */
  PWMOUT_Configuration();
  PWMOUT_Enable();

  return res;
}

/** \brief Initialize KeeLoq module (event timer and DMA transfer)
 *
 * \param
 * \param
 * \return
 *
 */
void KEELOQ_Configuration(void)
{
  TDmaSettings DmaSett;

  /**< Setup timer for DMA triggering */
  MCLK->APBCMASK.reg |= KEELOQ_DMA_TIMER_MCLK;
  GCLK->PCHCTRL[KEELOQ_DMA_TIMER_GCLK].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  KEELOQ_DMA_TIMER->COUNT8.CTRLA.bit.SWRST = 1;
  while (KEELOQ_DMA_TIMER->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);
  KEELOQ_DMA_TIMER->COUNT8.CTRLA.reg = (KEELOQ_TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | TC_CTRLA_MODE_COUNT8;
  KEELOQ_DMA_TIMER->COUNT8.CTRLBSET.reg = 0;
  KEELOQ_DMA_TIMER->COUNT8.EVCTRL.reg = TC_EVCTRL_OVFEO;
  KEELOQ_DMA_TIMER->COUNT8.PER.reg = KEELOQ_TIMER_VALUE;
  KEELOQ_DMA_TIMER->COUNT8.CTRLA.bit.ENABLE = 1;
  while (KEELOQ_DMA_TIMER->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_ENABLE);

  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
  DmaSett.trig_src = KEELOQ_DMA_OUT_ID;
  DmaSett.dst_addr = (void*)&PORT->Group[KEELOQ_PORT].OUTTGL.reg;
  DmaSett.src_addr = (void*)KEELOQ_Buff;
  DmaSett.src_inc = true;
  DmaSett.dst_inc = false;
  DmaSett.len = 126;
  DmaSett.priority = 0;
  DMA_SetupChannel(DMA_CHANNEL_KEELOQ, &DmaSett);

  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_WORD_Val;
  DmaSett.trig_src = KEELOQ_IN_DMA_ID;
  DmaSett.src_addr = (void*)&KEELOQ_IN_TIMER->CC[1].reg;
  DmaSett.dst_addr = (void*)KEELOQ_InBuff;
  DmaSett.src_inc = false;
  DmaSett.dst_inc = true;
  DmaSett.len = 128;
  DmaSett.priority = 0;
  DMA_SetupChannel(DMA_CHANNEL_IN_KEELOQ, &DmaSett);

//  GPIO_ClearPin(KEELOQ_PORT, KEELOQ_PIN);
//  GPIO_SetDir(KEELOQ_PORT, KEELOQ_PIN, true);
//  GPIO_SetFunction(KEELOQ_PORT, KEELOQ_PIN, GPIO_PIN_FUNC_OFF);

  /**< Setup power and clock for pin interrupt as event */
  GCLK->PCHCTRL[EIC_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  MCLK->APBAMASK.reg |= MCLK_APBAMASK_EIC;

  /**< Set pin event generation */
  if (EIC->CTRLA.bit.ENABLE == 0)
  {
    EIC->CTRLA.reg = EIC_CTRLA_SWRST;
    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_SWRST);
  } else
  {
    EIC->CTRLA.bit.ENABLE = 0;
    while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);
  }
  EIC->NMICTRL.reg |= (0 << EIC_NMICTRL_NMIFILTEN_Pos) | EIC_NMICTRL_NMISENSE(EIC_NMICTRL_NMISENSE_NONE_Val) | EIC_ASYNCH_ASYNCH(0);
  //EIC->INTENSET.reg |= (1 << KEELOQ_IN_PIN);
  EIC->EVCTRL.reg |= (1 << KEELOQ_IN_PIN);
  EIC->ASYNCH.reg |= (1 << KEELOQ_IN_PIN);
  EIC->DEBOUNCEN.reg |= (1 << KEELOQ_IN_PIN);
  EIC->DPRESCALER.reg |= 0;
  /**< Setup Ext.Pin 1 (CONFIG0) */
  EIC->CONFIG[0].reg |= (0 << EIC_CONFIG_FILTEN1_Pos) | EIC_CONFIG_SENSE1(EIC_NMICTRL_NMISENSE_HIGH);
  EIC->CONFIG[1].reg |= 0;
  EIC->CTRLA.reg |= EIC_CTRLA_ENABLE;
  while (EIC->SYNCBUSY.reg & EIC_SYNCBUSY_ENABLE);

//  /**< Initialize interrupts */
//  NVIC_DisableIRQ(EIC_IRQn);
//  NVIC_ClearPendingIRQ(EIC_IRQn);
//  NVIC_EnableIRQ(EIC_IRQn);


  /**< Setup timer for input capture */
  MCLK->APBCMASK.reg |= KEELOQ_IN_TIMER_MCLK;
  GCLK->PCHCTRL[KEELOQ_IN_TIMER_GCLK].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

//  /**< Reset TCC module */
//  KEELOQ_IN_TIMER->CTRLA.reg = TCC_CTRLA_SWRST;
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_SWRST);
//  KEELOQ_IN_TIMER->CTRLBCLR.reg = TCC_CTRLBCLR_DIR;     /* count up */
//  while (KEELOQ_IN_TIMER->SYNCBUSY.reg & TCC_SYNCBUSY_CTRLB);
//
//  /**< configure the TCC device */
//  KEELOQ_IN_TIMER->CTRLA.reg = (TCC_CTRLA_PRESCSYNC_GCLK_Val | TCC_CTRLA_PRESCALER(TCC_CTRLA_PRESCALER_DIV2_Val) | TCC_CTRLA_CPTEN0 | TCC_CTRLA_CPTEN1);
//  KEELOQ_IN_TIMER->EVCTRL.reg = (TCC_EVCTRL_MCEI0 | TCC_EVCTRL_MCEI1 | TCC_EVCTRL_TCEI1 | TCC_EVCTRL_EVACT1_PPW);


  /**< Setup event system */
  GCLK->PCHCTRL[EVSYS_GCLK_ID_0].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_EVSYS;
  /**< Event channel 0 setup */
  EVSYS->CHANNEL[0].reg = (1 << EVSYS_CHANNEL_ONDEMAND_Pos) | (0 << EVSYS_CHANNEL_RUNSTDBY_Pos)
                       | EVSYS_CHANNEL_EDGSEL(EVSYS_CHANNEL_EDGSEL_BOTH_EDGES_Val) | EVSYS_CHANNEL_PATH(EVSYS_CHANNEL_PATH_ASYNCHRONOUS_Val)
                       | EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_EIC_EXTINT_1);
  /* <23=>TCC2_EV_1 as event destination, <0x1=>Channel 0 */
  EVSYS->USER[KEELOQ_EVSYS_TIMER_ID].reg = 1;

//  GPIO_ClearPin(KEELOQ_IN_PORT, KEELOQ_IN_PIN);
//  GPIO_SetDir(KEELOQ_IN_PORT, KEELOQ_IN_PIN, false);
//  GPIO_SetPullMode(KEELOQ_IN_PORT, KEELOQ_IN_PIN, GPIO_PULLMODE_UP);
//  GPIO_SetFunction(KEELOQ_IN_PORT, KEELOQ_IN_PIN, MUX_PA10A_EIC_EXTINT10);
}


