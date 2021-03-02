#ifndef JDY08_H
#define JDY08_H

#include "defines.h"

#define JDY08_RETRIES    3

typedef struct
{
  const char *cmd;
  void *param;
  const char *reply;
  bool  waitReply;
  uint16_t timeout;
  uint16_t delay;
} tAtCmd;

void JDY08_Configuration(void);

#endif
