#ifndef _PARSER_H
#define _PARSER_H

#include "defines.h"

enum PARSER_COMMANDS
{
  PARSER_CMD_ID,
  PARSER_CMD_VN,
  PARSER_CMD_HW,

  PARSER_CMD_GB,
  PARSER_CMD_GCI,
  PARSER_CMD_GCP,
  PARSER_CMD_GP,
  PARSER_CMD_GT,
  PARSER_CMD_GUS,
  PARSER_CMD_GU,

  PARSER_CMD_SB,
  PARSER_CMD_SI,

  PARSER_CMD_DP,

  PARSER_CMD_CC,
  PARSER_CMD_CUS,

  PARSER_CMD_RB,
  PARSER_CMD_WB,

  PARSER_CMD_LED,
  PARSER_CMD_ADC,
  PARSER_CMD_PWR,
  PARSER_CMD_OUT,

  PARSER_CMD_BLS,
  PARSER_CMD_BLQ,
  PARSER_CMD_BLF,
  PARSER_CMD_BLC,
  PARSER_CMD_BLE,

  PARSER_CMD_PSW,

  PARSER_CMD_PAUSE,

  PARSER_CMD_UPD,

  PARSER_CMD_LAST
};

#define PARSER_SRC_COMM   0x01
#define PARSER_SRC_TEXT   0x02
#define PARSER_SRC_ALL    (PARSER_SRC_COMM | PARSER_SRC_TEXT)

/**< parsed item structure */
typedef struct
{
	// string
	char const *itemStr;
	// format
	char const *inFmt;
	// output message format
	char const *outFmt;
	// help string
	char const *help;
	// define the possible source of the command
	uint8_t source;
	// needs authorization
	bool secure;
} TParsedItem;

typedef TParsedItem TParsedItems[];

const TParsedItem PARSER_Items[PARSER_CMD_LAST];

void PARSER_Process(char *cmd, char *buff, uint8_t source);

#endif
