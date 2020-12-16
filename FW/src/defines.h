#ifndef DEFINES_H
#define DEFINES_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include "samc21e18a.h"
#include "peripheral_clk_config.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define INIT_TASK_STACK_SIZE      (200)
#define MAIN_TASK_STACK_SIZE      (200)
#define MONITOR_TASK_STACK_SIZE   (100)
#define MOTOR_TASK_STACK_SIZE     (200)
#define PWM_TASK_STACK_SIZE       (100)
#define CAN_TASK_STACK_SIZE       (100)
#define RS485_TASK_STACK_SIZE     (100)

#define MAIN_TASK_PRIORITY     (tskIDLE_PRIORITY + 2)
#define MONITOR_TASK_PRIORITY  (tskIDLE_PRIORITY + 1)
#define COMM_TASK_PRIORITY     (tskIDLE_PRIORITY + 3)
#define MOTOR_TASK_PRIORITY    (tskIDLE_PRIORITY + 4)
//#define HALL_TASK_PRIORITY     (tskIDLE_PRIORITY + 5)
#define INIT_TASK_PRIORITY     (tskIDLE_PRIORITY + 5)

enum COMM_TYPE
{
  COMM_TYPE_PWM,
  COMM_TYPE_RS485,
  COMM_TYPE_CAN,
  COMM_TYPE_LAST
};

/**< Maximal value for device ID on CAN bus */
#define MAX_ID_VALUE            0x1f

#define MIN_TP                  (-1934)       // -170°
#define MAX_TP                  (1934)        // +170°

#define OPTIONS_RESET           0x00        // options bits
#define OPTIONS_TERMINATION_ON  (1 << 0)
#define OPTIONS_REVERSE_ON      (1 << 1)

#define CANBUS_DFLT_ID          0x3F0

#define SERVO_NAME              "DA15-C"
#define SERVO_NAME_LEN          6

enum {
  PARAM_TYPE_BOOL,
  PARAM_TYPE_UINT8,
  PARAM_TYPE_INT8,
  PARAM_TYPE_UINT16,
  PARAM_TYPE_INT16,
  PARAM_TYPE_UINT32,
  PARAM_TYPE_INT32,
  PARAM_TYPE_FLOAT16,
  PARAM_TYPE_FLOAT32,
  PARAM_TYPE_STRING
};

#define PARAM_NAME_LEN          6

#define TEST_BOARD

#endif
