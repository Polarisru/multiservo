#ifndef COMM_H
#define COMM_H

#include "defines.h"

#define COMM_BUFFER_SIZE      RS232_BUFFER_SIZE

void COMM_SetSecured(bool on);
void COMM_Task(void *pParameters);

#endif
