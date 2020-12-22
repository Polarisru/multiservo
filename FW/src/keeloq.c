#include "driver_gpio.h"
#include "driver_dma.h"
#include "keeloq.h"

enum {
  KEELOQ_MODE_SEND,
  KEELOQ_MODE_REQUEST
};

#define DMA_CHANNEL_KEELOQ    0

#define KEELOQ_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV64_Val
#define KEELOQ_TIMER_VALUE    (CONF_GCLK_TC0_FREQUENCY / 1000000UL * KEELOQ_BIT_LEN / 64)

#define KEELOQ_PORT           GPIO_PORTA
#define KEELOQ_PIN            10

#define KEELOQ_BIT_LEAVE      0
#define KEELOQ_BIT_TOGGLE     (1 << KEELOQ_PIN)

#define KEELOQ_RETRIES        3         // number of retries

uint32_t KEELOQ_Buff[128];
uint8_t KEELOQ_Pos, KEELOQ_Last;
uint8_t KEELOQ_Mode;
static bool KEELOQ_RxReady;

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
			byte = *request++;
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
  DMA_StartChannel(DMA_CHANNEL_KEELOQ);

  vTaskDelay(30);

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

  //TC2->COUNT8.INTENSET.reg = TC_INTENSET_OVF;

  //NVIC_EnableIRQ(TC2_IRQn);

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
  DMA_SetupChannel(0, &DmaSett);

  GPIO_ClearPin(KEELOQ_PORT, KEELOQ_PIN);
  GPIO_SetDir(KEELOQ_PORT, KEELOQ_PIN, true);
  GPIO_SetFunction(KEELOQ_PORT, KEELOQ_PIN, GPIO_PIN_FUNC_OFF);
}


