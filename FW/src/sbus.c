#include "drivers.h"
#include "global.h"
#include "outputs.h"
#include "sbus.h"
#include "serial.h"

#define SBUS_CHANNEL       SERCOM1

#define SBUS_DMA_TRIG      SERCOM1_DMAC_ID_TX

#define SBUS_BAUDRATE      100000UL

static uint16_t SBUS_Values[SBUS_MAX_CHANNEL];
static uint8_t SBUS_Data[SBUS_MAX_DATALEN];
static uint8_t SBUS_Flags;

//void SBUS_Send(uint8_t *data, uint8_t len)
//{
//  while (len-- > 0)
//  {
//    UART_SendByte(SBUS_CHANNEL, *data++);
//  }
//}

/** \brief Set channel value
 *
 * \param [in] channel Number of SBUS channel
 * \param [in] value Value to set (11 bit)
 * \return Nothing
 *
 */
void SBUS_SetChannel(uint8_t channel, uint16_t value)
{
  if (channel >= SBUS_MAX_CHANNEL)
    return;

  SBUS_Values[channel] = value & SBUS_VALUE_MASK;
}

/** \brief Set new position to ALL channels
 *
 * \param [in] position Position to set
 * \return True if succeed (always)
 *
 */
bool SBUS_SetPosition(float position)
{
  uint8_t i;
  uint16_t value;

  if (position < SBUS_MIN_POSITION)
    position = SBUS_MIN_POSITION;
  if (position > SBUS_MAX_POSITION)
    position = SBUS_MAX_POSITION;

  value = (uint16_t)((position - SBUS_MIN_POSITION) * SBUS_MAX_VALUE / (SBUS_MAX_POSITION - SBUS_MIN_POSITION));
  if (value >= SBUS_MAX_VALUE)
    value = SBUS_MAX_VALUE - 1;

  for (i = 0; i < SBUS_MAX_CHANNEL; i++)
    SBUS_SetChannel(i, value);

  return true;
}

/** \brief Send SBUS command (all servos are commanded simultaneously)
 *
 * \return Nothing
 *
 */
void SBUS_SendCmd(void)
{
  uint8_t i;
  uint8_t len = SBUS_DATA_LEN;

  memset(SBUS_Data, 0, SBUS_DATA_LEN);
  SBUS_Data[SBUS_OFFS_CMD] = SBUS_CMD_START;
  for (i = 0; i < SBUS_MAX_CHANNEL * SBUS_DATA_BITS; i++)
  {
    if (SBUS_Values[i / SBUS_DATA_BITS] & (1 << (i % SBUS_DATA_BITS)))
      SBUS_Data[SBUS_OFFS_DATA + (i / 8)] |= (1 << (i % 8));
  }
  SBUS_Data[SBUS_OFFS_FLAGS] = SBUS_Flags;
  SBUS_Data[len - 1] = SBUS_CMD_STOP;

  //SBUS_Send(SBUS_Data, len);
  DMA_StartChannel(DMA_CHANNEL_SBUS);
}

/** \brief Enable SBUS
 *
 * \return Nothing
 *
 */
void SBUS_Enable(void)
{
  /**< Setup TX/RX pins */
  SERIAL_Enable();
  /**< Configure outputs */
  OUTPUTS_Switch(OUTPUTS_SBUSPOL, OUTPUTS_SWITCH_ON);
  OUTPUTS_Switch(OUTPUTS_SBUSTE, OUTPUTS_SWITCH_ON);
  /**< Resume SBUS task */
  vTaskResume(xTaskSbus);
}

/** \brief Disable SBUS
 *
 * \return Nothing
 *
 */
void SBUS_Disable(void)
{
  /**< Disable pins */
  SERIAL_Disable();
  OUTPUTS_Switch(OUTPUTS_SBUSPOL, OUTPUTS_SWITCH_OFF);
  /**< Suspend SBUS task */
  vTaskSuspend(xTaskSbus);
}

/** \brief RTOS task for processing SBUS transmission
 */
void SBUS_Task(void *pParameters)
{
  (void) pParameters;   /* to quiet warnings */

  while (1)
  {
    vTaskDelay(pdMS_TO_TICKS(20));
    SBUS_SendCmd();
  }
}

/** \brief Configure SBUS (UART, DMA)
 *
 * \return Nothing
 *
 */
void SBUS_Configuration(void)
{
  uint8_t i;
  TDmaSettings DmaSett;

  for (i = 0; i < SBUS_MAX_CHANNEL; i++)
    SBUS_Values[i] = 0;
  SBUS_Flags = 0x03;

  /**< Configure DMA channel for SBUS transmission */
  DmaSett.beat_size = DMAC_BTCTRL_BEATSIZE_BYTE_Val;
  DmaSett.trig_src = SBUS_DMA_TRIG;
  DmaSett.src_addr = (void*)SBUS_Data;
  DmaSett.dst_addr = (void*)&SBUS_CHANNEL->USART.DATA.reg;
  DmaSett.src_inc = true;
  DmaSett.dst_inc = false;
  DmaSett.len = SBUS_MAX_DATALEN;
  DmaSett.priority = 0;
  DMA_SetupChannel(DMA_CHANNEL_SBUS, &DmaSett);
}
