#ifndef COMM_H
#define COMM_H

#include "defines.h"

#define COMM_BUFFER_SIZE      RS232_BUFFER_SIZE

enum {
  COMM_MODE_AT,
  COMM_MODE_NORMAL,
  COMM_MODE_LAST
};

void COMM_SetSecured(bool on);
void COMM_SetMode(uint8_t mode);
void COMM_Task(void *pParameters);

#endif
