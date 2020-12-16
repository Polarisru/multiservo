#include "keeloq.h"
#include "driver_dma.h"
#include "driver_capture.h"

#define DMA_CHANNEL_KEELOQ    0

#define KEELOQ_TIMER_PRESCALER  TC_CTRLA_PRESCALER_DIV64_Val
#define KEELOQ_TIMER_VALUE    (CONF_GCLK_TC0_FREQUENCY / 1000000UL * KEELOQ_BIT_LEN / 64)

#define KEELOQ_BIT_LEAVE      0
#define KEELOQ_BIT_TOGGLE     (1 << CAPTURE_PIN)

uint32_t KEELOQ_Buff[128];
uint8_t KEELOQ_Pos, KEELOQ_Last;

void TC2_Handler(void)
{
  TC2->COUNT8.INTFLAG.reg = TC_INTFLAG_OVF;
  PORT->Group[CAPTURE_PORT].OUTTGL.reg = KEELOQ_Buff[KEELOQ_Pos++];
  if (KEELOQ_Pos >= KEELOQ_Last)
    TC2->COUNT8.CTRLA.bit.ENABLE = 0;
}

void KEELOQ_Send(uint8_t *data)
{
	uint8_t i, j;
	uint8_t pos = 0;
	uint8_t byte;
	uint8_t crc = 0;

  CAPTURE_Disable();
  /**< Configure PWM pin as output */
  CAPTURE_DisablePin();

	/**< Set preamble */
  KEELOQ_Buff[pos++] = KEELOQ_BIT_LEAVE;
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
			byte = *data++;
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

	KEELOQ_Last = pos;
	KEELOQ_Pos = 0;

  //DMA_StartChannel(DMA_CHANNEL_KEELOQ, true);
  TC2->COUNT8.CTRLA.bit.ENABLE = 1;
  while (TC2->COUNT8.SYNCBUSY.reg & (TC_SYNCBUSY_SWRST | TC_SYNCBUSY_ENABLE));
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
  /**< Setup timer as events source */
  MCLK->APBCMASK.reg |= MCLK_APBCMASK_TC2;
  GCLK->PCHCTRL[TC2_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK0_Val | (1 << GCLK_PCHCTRL_CHEN_Pos);

  TC2->COUNT8.CTRLA.bit.SWRST = 1;
  while (TC2->COUNT8.SYNCBUSY.reg & TC_SYNCBUSY_SWRST);

  TC2->COUNT8.CTRLA.reg = (KEELOQ_TIMER_PRESCALER << TC_CTRLA_PRESCALER_Pos) | TC_CTRLA_MODE_COUNT8;
  TC2->COUNT8.CTRLBSET.reg = 0;
  TC2->COUNT8.EVCTRL.reg = TC_EVCTRL_OVFEO;
  TC2->COUNT8.PER.reg = KEELOQ_TIMER_VALUE;

  TC2->COUNT8.INTENSET.reg = TC_INTENSET_OVF;

  NVIC_EnableIRQ(TC2_IRQn);
}


