#include <ctype.h>
#include <string.h>
#include "comm.h"
#include "jdy08.h"
#include "parser.h"
#include "rs232.h"
#include "utils.h"

const char commHelp[] = "HELP";

static bool COMM_Secured;
static uint8_t COMM_Mode;

/** \brief Send answer
 *
 * \param [in] buffer Data to send
 * \return Nothing
 *
 */
void COMM_Send(char *buffer)
{
  uint8_t len = strlen(buffer);

  RS232_SendData((uint8_t*)buffer, len);
}

/** \brief Set secured status
 *
 * \param [in] on True to set secured, false otherwise
 * \return Nothing
 *
 */
void COMM_SetSecured(bool on)
{
  COMM_Secured = on;
}

void COMM_SetMode(uint8_t mode)
{
  if (mode >= COMM_MODE_LAST)
    return;

  COMM_Mode = mode;
}

/** \brief RTOS task for processing UART connection
 */
void COMM_Task(void *pParameters)
{
  (void) pParameters;   /* to quiet warnings */
  char buff[128];
  uint16_t pos;
  char *cmd;

  /**< Configure UART connection */
  RS232_Configuration();
  /**< Initial settings */
  COMM_Secured = false;
  COMM_Mode = COMM_MODE_NORMAL;
  /**< Configure JDY-08 module */
  JDY08_Configuration();

  while (1)
  {
    /**< Wait for notification from the RX interrupt */
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    cmd = RS232_RxBuffer;
    pos = 0;
    while ((CHAR_CR != cmd[pos]) && (CHAR_NL != cmd[pos]) && cmd[pos])
      pos++;

    if (pos >= COMM_BUFFER_SIZE)
      pos = COMM_BUFFER_SIZE - 1;
    cmd[pos] = '\0';

    /**< Find command - skip blanks */
    while (*cmd && isspace(*cmd))
      cmd++;
    pos = 0;
    while (cmd[pos] && (!isspace(cmd[pos])))
      pos++;
    cmd[pos] = '\0';

    UTILS_StrUpr(cmd);

    if (COMM_Mode == COMM_MODE_AT)
    {

    } else
    {
      if (0 == strncmp(cmd, commHelp, strlen(commHelp)))
      {
        /**< It is a HELP command, show help screen */
        for (pos = 0; pos < PARSER_CMD_LAST; pos++)
        {
          if ((PARSER_Items[pos].source & PARSER_SRC_COMM) && ((PARSER_Items[pos].secure == false) || (COMM_Secured == true)))
          {
            /**< Show only communication commands */
            strcpy(buff, PARSER_Items[pos].itemStr);
            strcat(buff, LINE_SEPARATE);
            strcat(buff, PARSER_Items[pos].help);
            strcat(buff, LINE_CRLF);
            COMM_Send(buff);
          }
        }
      } else
      {
        /**< It is not a HELP command, go to parser */
        PARSER_Process(cmd, buff, PARSER_SRC_COMM);
        COMM_Send(buff);
      }
    }
  }
}
