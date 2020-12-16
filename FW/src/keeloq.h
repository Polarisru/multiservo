#ifndef KEELOQ_H
#define KEELOQ_H

#include "defines.h"

#define KEELOQ_BIT_LEN			  200			  // bit length in useconds
#define KEELOQ_HEADER_LEN		  2000		  // header length in useconds
#define KEELOQ_HEADER_MAXLEN  2600      // maximal length for KeeLoq header in useconds
#define KEELOQ_HEADER_MINLEN  1600      // minimal length for KeeLoq header in useconds
#define KEELOQ_BIT_MAXDIFF    40        // maximal time difference for bit recognition in useconds
#define KEELOQ_BIT0_MAXLEN    (KEELOQ_BIT_LEN + KEELOQ_BIT_MAXDIFF)
#define KEELOQ_BIT0_MINLEN    (KEELOQ_BIT_LEN - KEELOQ_BIT_MAXDIFF)
#define KEELOQ_BIT1_MAXLEN    (KEELOQ_BIT_LEN * 2 + KEELOQ_BIT_MAXDIFF)
#define KEELOQ_BIT1_MINLEN    (KEELOQ_BIT_LEN * 2 - KEELOQ_BIT_MAXDIFF)

#define KEELOQ_PACKET_LEN		  3				  // packet length

#define KEELOQ_PREAMBLE_LEN	  20	      // number of preamble pulses

void KEELOQ_Send(uint8_t *data);
void KEELOQ_Init(void);

#endif
