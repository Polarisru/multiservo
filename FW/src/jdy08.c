#include "drivers.h"
#include "jdy08.h"
#include "rs232.h"

#define JDY08_PORT          GPIO_PORTA
#define JDY08_PIN_RES       22
#define JDY08_PIN_PWRC      19

static char JDY08_cmd[96];

const char JDY08_NL[] = "\r\n";

void JDY08_SendCmd(char *cmd)
{
  uint8_t len = strlen(cmd);
  RS232_SendData((uint8_t*)cmd, len);
}

bool JDY08_ProcessCmd(tAtCmd *cmd)
{
  uint8_t i = 0, j;

  while (i < JDY08_RETRIES)
  {
    if (cmd->param != NULL)
    {
      sprintf(JDY08_cmd, cmd->cmd, cmd->param);
    } else
    {
      strcpy(JDY08_cmd, cmd->cmd);
    }
    strcat(JDY08_cmd, JDY08_NL);
    //DELAY_DoMs(100);
    JDY08_SendCmd(JDY08_cmd);
    if (cmd->reply == NULL)
      return true;
    j = 0;
    //DELAY_SetTimeout(cmd->timeout);
//    while ((j < SIM800_RETRIES) && (DELAY_IsTimeout() == false))
//    {
//      if (UART_IsRxReady(UART_NUM1) == true)
//      {
//        strcpy(JDY08_cmd, (char*)UART_RxBuffer);
//        /**< Analyze reply */
//        if (strncmp(JDY08_cmd, cmd->reply, strlen(cmd->reply)) == 0)
//          return true;
//        j++;
//      }
//    }
    i++;
  }

  return false;
}

void JDY08_Configuration(void)
{
  GPIO_SetFunction(JDY08_PORT, JDY08_PIN_RES, GPIO_PIN_FUNC_OFF);
  GPIO_SetFunction(JDY08_PORT, JDY08_PIN_PWRC, GPIO_PIN_FUNC_OFF);
  GPIO_SetDir(JDY08_PORT, JDY08_PIN_RES, true);
  GPIO_SetDir(JDY08_PORT, JDY08_PIN_PWRC, true);
  GPIO_SetPin(JDY08_PORT, JDY08_PIN_RES);
  GPIO_SetPin(JDY08_PORT, JDY08_PIN_PWRC);
}
