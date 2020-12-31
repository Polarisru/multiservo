#ifndef DRV_CAN_H
#define DRV_CAN_H

#include "defines.h"
#include "err_codes.h"

/**< Baudrates */
enum {
  CAN_BAUD_125K,
  CAN_BAUD_250K,
  CAN_BAUD_500K,
  CAN_BAUD_1M,
  CAN_BAUD_2M,
  CAN_BAUD_4M,
  CAN_BAUD_LAST
};

/**
 * \brief CAN Message Format
 */
enum can_format {
	CAN_FMT_STDID, /*!< Standard Format, 11 bits identifier */
	CAN_FMT_EXTID  /*!< Extended Format, 29 bits identifier */
};

/**
 * \brief CAN Message Type
 */
enum can_type {
	CAN_TYPE_DATA,  /*!< A DATA FRAME carries data from a transmitter to the
	                    receivers.*/
	CAN_TYPE_REMOTE, /*!< A REMOTE FRAME is transmitted by a bus unit to request
	                     the transmission of the DATA FRAME with the same
	                     IDENTIFIER */
  CAN_TYPE_FD,
  CAN_TYPE_FD_BRS
};

/**
 * \brief CAN Filter
 */
struct can_filter {
	uint32_t id;   /* Message identifier */
	uint32_t mask; /* The mask applied to the id */
};

/**
 * \brief CAN Message
 */
struct can_message {
	uint32_t        id;   /* Message identifier */
	enum can_type   type; /* Message Type */
	uint8_t *       data; /* Pointer to Message Data */
	uint8_t         len;  /* Message Length */
	enum can_format fmt;  /* Identifier format, CAN_STD, CAN_EXT */
};

bool CAN_HasMessage(void);
int32_t CAN_ReadMsg(Can *channel, struct can_message *msg);
int32_t CAN_WriteMsg(Can *channel, struct can_message *msg);
int32_t CAN_SetFilter(uint8_t index, enum can_format fmt, struct can_filter *filter);
int32_t CAN_SetRangeFilter(uint8_t index, enum can_format fmt, uint32_t start_id, uint32_t stop_id);
void CAN_Enable(Can *channel);
void CAN_Disable(Can *channel);
bool CAN_SetBaudrate(Can *channel, uint32_t nominal_baudrate, uint32_t data_baudrate);
void CAN_SetFD(Can *channel, bool on);
void CAN_SetBRS(Can *channel, bool on);
void CAN_CheckBus(Can *channel);
void CAN_Init(Can *channel);

#endif

