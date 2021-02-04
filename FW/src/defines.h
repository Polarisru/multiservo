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

#include "drivers.h"

#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#define INIT_TASK_STACK_SIZE      (300)
#define MAIN_TASK_STACK_SIZE      (200)
#define SBUS_TASK_STACK_SIZE      (100)
#define COMM_TASK_STACK_SIZE      (500)

#define INIT_TASK_PRIORITY     (tskIDLE_PRIORITY + 5)
#define MAIN_TASK_PRIORITY     (tskIDLE_PRIORITY + 2)
#define SBUS_TASK_PRIORITY     (tskIDLE_PRIORITY + 1)
#define COMM_TASK_PRIORITY     (tskIDLE_PRIORITY + 3)

#define CHAR_CR 	              '\r'
#define CHAR_NL		              '\n'
#define CHAR_ESC 	              0x1B
#define CHAR_BSPACE             0x08
#define LINE_CRLF               "\n"
#define LINE_SEPARATE           " - "

#define M_PI     3.1416f

enum CONN_MODE
{
  CONN_MODE_PWM,
  CONN_MODE_RS485,
  CONN_MODE_CAN,
  CONN_MODE_AMAZON,
  CONN_MODE_SBUS,
  CONN_MODE_UAVCAN_0,
  CONN_MODE_UAVCAN_1,
  CONN_MODE_LAST
};

enum {
  ADC_TYPE_CURRENT,
  ADC_TYPE_MAX,
  ADC_TYPE_LAST
};

#define UPD_PASSWORD            1234
#define PASSWORD_SECURED        1234

/**< Maximal value for device ID on CAN bus */
#define MAX_ID_VALUE            0x1f

#define MIN_TP                  (-1934)       // -170°
#define MAX_TP                  (1934)        // +170°

#define SERVO_RAW_ANGLE_SCALE   0x800
#define SERVO_ANGLE_SCALE       180.0f    // degrees
#define SERVO_MAX_ANGLE         170.0f    // maximal angle in degrees

#define PWM_RANGE               90 // -45°..45°

#define OPTIONS_RESET           0x00        // options bits
#define OPTIONS_TERMINATION_ON  (1 << 0)
#define OPTIONS_REVERSE_ON      (1 << 1)

#define CANBUS_DFLT_ID          0x3F0

#define EG_BIT_RS485_RX         (1 << 0)
#define EG_BIT_CAN_RX           (1 << 1)

#define KEELOQ_ADDR_MAGSENSOR   0x62
#define KEELOQ_ADDR_TEMP        0x27
#define KEELOQ_ADDR_VOLTAGE     0x39
#define KEELOQ_ADDR_CURRENT     0x3A

enum {
  DMA_CHANNEL_KEELOQ,
  DMA_CHANNEL_IN_KEELOQ,
  DMA_CHANNEL_IN_KEELOQ2,
  DMA_CHANNEL_ADC_I,
  DMA_CHANNEL_SBUS,
  DMA_CHANNELS_NUM
};

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
