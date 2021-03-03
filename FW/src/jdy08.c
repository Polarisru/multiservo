#include "drivers.h"
#include "jdy08.h"
#include "rs232.h"

#define JDY08_PORT          GPIO_PORTA
#define JDY08_PIN_RES       22
#define JDY08_PIN_PWRC      19

const char JDY08_NL[] = "\r\n";
const char JDY08_CMD_AT[] = "AT";
const char JDY08_CMD_NAME[] = "AT+NAME%s";

const char JDY08_ANS_OK[] = "+OK";

const char JDY08_NAME[] = "ServoTester";

static char JDY08_cmd[96];
static char JDY08_param[64];


tAtCmd JDY08_cmdSetName[] = {
                      {JDY08_CMD_NAME, (void*)JDY08_param, JDY08_ANS_OK, true, 500, 100},
                      {NULL}
                    };

void JDY08_SendCmd(char *cmd)
{
  uint8_t len = strlen(cmd);
  RS232_ResetRx();
  RS232_SendData((uint8_t*)cmd, len);
}

bool JDY08_ProcessCmd(tAtCmd *cmd)
{
  uint8_t i = 0, j;
  uint32_t ticks;

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
    JDY08_SendCmd(JDY08_cmd);
    if (cmd->reply == NULL)
      return true;
    j = 0;
    ticks = xTaskGetTickCount() + cmd->timeout;
    while ((j < JDY08_RETRIES) && (xTaskGetTickCount() < ticks))
    {
      if (ulTaskNotifyTake(pdTRUE, 0) == pdTRUE)
      {
        strcpy(JDY08_cmd, (char*)RS232_RxBuffer);
        /**< Analyze reply */
        if (strncmp(JDY08_cmd, cmd->reply, strlen(cmd->reply)) == 0)
          return true;
        j++;
      }
    }
    i++;
    vTaskDelay(50);
  }

  return false;
}

bool JDY08_SetName(char *name)
{
  tAtCmd *cmd = JDY08_cmdSetName;

  strcpy(JDY08_param, name);

  while (cmd->cmd != NULL)
  {
    if (JDY08_ProcessCmd(cmd) == false)
      return false;
    cmd++;
  }

  return true;
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
