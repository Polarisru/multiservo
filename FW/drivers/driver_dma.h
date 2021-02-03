#ifndef DRV_DMA_H
#define DRV_DMA_H

#include "defines.h"

typedef struct
{
  void      *dst_addr;
  void      *src_addr;
  uint16_t  len;
  uint8_t   beat_size;
  uint8_t   trig_src;
  uint8_t   priority;
  bool      dst_inc;
  bool      src_inc;
} TDmaSettings;

void DMA_Init(void);
void DMA_SetupChannel(uint8_t channel, TDmaSettings *settings);
void DMA_StartChannel(uint8_t channel);
void DMA_StopChannel(uint8_t channel);
bool DMA_IsReady(uint8_t channel);

#endif
