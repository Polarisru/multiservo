#include "global.h"
#include "uavcan.h"
#include "driver_wdt.h"

#define PUBLISHER_PERIOD_mS     100
#define SIGNALTEST_PERIOD_mS    1000

/**< Arena for memory allocation, used by the library */
uint8_t g_canard_memory_pool[1024];
/**< The libcanard library instance */
static CanardInstance g_canard;
/**< Transport stats counters */
uint64_t counterTx, counterRx, counterErr;
bool rawcommandTag = false;
uint16_t rawcommandTransfer = 0;

/**< Parameters we can adjust using UAVCAN interface */
param_t parameters[] =
{
  /* name      value  min max default*/
  {"NodeID",      0,  0,  100,   0},
  {"P-gain",      30, 1,  255,  30},
  {"D-gain",      60, 1,  255,   60}
};

void restartHandleCanard(CanardRxTransfer* transfer)
{
  uint64_t value;

  canardDecodeScalar(transfer, 0, 40, false, &value);
  if (value == UAVCAN_PROTOCOL_RESTART_MAGICNUM)
  {
    WDT_Init();
    WDT_Enable();
  }
}

void getTransStatsHandleCanard(CanardRxTransfer* transfer)
{
  uint8_t  buffer[64] = "";
  uint16_t len;

  canardEncodeScalar(buffer, 0, 48, &counterTx);
  canardEncodeScalar(buffer, 48, 48, &counterRx);
  canardEncodeScalar(buffer, 96, 48, &counterErr);
  len = 18;
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_PROTOCOL_GET_TRANSSTATS_SIGNATURE,
                                      UAVCAN_PROTOCOL_GET_TRANSSTATS_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);
}

void setActuatorHandleCanard(CanardRxTransfer* transfer)
{
  uint16_t len = 0;
  uint8_t id, type;
  uint16_t value;
  float angle;

  /**< Parse array command for every actuator */
  for (len = 0; len < transfer->payload_len; len += UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_LEN)
  {
    canardDecodeScalar(transfer, len, 8, false, &id);
    if (id == EE_Id)
    {
      /**< Own ID found in array */
      canardDecodeScalar(transfer, len + 8, 8, false, &type);
      if (type == UAVCAN_EQUIPMENT_ACTUATOR_COMMAND_TYPE_POSITION)
      {
        canardDecodeScalar(transfer, len + 16, 16, false, &value);
        /**< Convert float16 value in radians to angle position */
        angle = canardConvertFloat16ToNativeFloat(value) * 180 / PI;


        uint8_t  buffer[64] = "";
        canardEncodeScalar(buffer, 0, 8, &EE_Id);
        canardEncodeScalar(buffer, 8, 16, &value);
        value = UAVCAN_FLOAT16_NAN;
        canardEncodeScalar(buffer, 24, 16, &value);
        canardEncodeScalar(buffer, 40, 16, &value);
        type = 127;
        canardEncodeScalar(buffer, 56, 8, &type);
        len = 8;
        //uint8 actuator_id           # Generic actuator feedback, if available.
        //float16 position            # meter or radian
        //float16 force               # Newton or Newton metre
        //float16 speed               # meter per second or radian per second
        //void1
        //uint7 POWER_RATING_PCT_UNKNOWN = 127
        //uint7 power_rating_pct      # 0 - unloaded, 100 - full load
        /**< Send servo stat as reply */
        int result = canardRequestOrRespond(&g_canard,
                                            transfer->source_node_id,
                                            UAVCAN_EQUIPMENT_ACTUATOR_STATUS_SIGNATURE,
                                            UAVCAN_EQUIPMENT_ACTUATOR_STATUS_ID,
                                            &transfer->transfer_id,
                                            transfer->priority,
                                            CanardResponse,
                                            &buffer[0],
                                            (uint16_t)len);
      }
    }
  }
}

bool UAVCAN_shouldAcceptTransfer(const CanardInstance* ins,
                          uint64_t* out_data_type_signature,
                          uint16_t data_type_id,
                          CanardTransferType transfer_type,
                          uint8_t source_node_id)
{
  if ((transfer_type == CanardTransferTypeRequest) && (data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
  {
    *out_data_type_signature = UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID)
  {
    *out_data_type_signature = UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_RESTART_NODE_ID)
  {
    *out_data_type_signature = UAVCAN_PROTOCOL_RESTART_NODE_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_PROTOCOL_GET_TRANSSTATS_ID)
  {
    *out_data_type_signature = UAVCAN_PROTOCOL_GET_TRANSSTATS_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID)
  {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_SIGNATURE;
    return true;
  }
  if (data_type_id == UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID)
  {
    *out_data_type_signature = UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_SIGNATURE;
    return true;
  }
  return false;
}

void UAVCAN_onTransferReceived(CanardInstance* ins, CanardRxTransfer* transfer)
{
  if ((transfer->transfer_type == CanardTransferTypeRequest) && (transfer->data_type_id == UAVCAN_GET_NODE_INFO_DATA_TYPE_ID))
  {
    getNodeInfoHandleCanard(transfer);
  }
  if (transfer->data_type_id == UAVCAN_PROTOCOL_PARAM_GETSET_ID)
  {
    getsetHandleCanard(transfer);
  }
  if (transfer->data_type_id == UAVCAN_PROTOCOL_RESTART_NODE_ID)
  {
    restartHandleCanard(transfer);
  }
  if (transfer->data_type_id == UAVCAN_PROTOCOL_GET_TRANSSTATS_ID)
  {
    getTransStatsHandleCanard(transfer);
  }
  if (transfer->data_type_id == UAVCAN_EQUIPMENT_ESC_RAWCOMMAND_ID)
  {
    rawcmdHandleCanard(transfer);
  }
  if (transfer->data_type_id == UAVCAN_EQUIPMENT_ACTUATOR_ARRAYCOMMAND_ID)
  {
    setActuatorHandleCanard(transfer);
  }
}

void getNodeInfoHandleCanard(CanardRxTransfer* transfer)
{
  uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE];
  memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  uint16_t len = UAVCAN_MakeNodeInfoMessage(buffer);
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_SIGNATURE,
                                      UAVCAN_GET_NODE_INFO_DATA_TYPE_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);
}

void UAVCAN_Init(void)
{
  CANBUS_Init();

  counterTx = 0;
  counterRx = 0;
  counterErr = 0;

  canardInit(&g_canard,                         // Uninitialized library instance
             g_canard_memory_pool,              // Raw memory chunk used for dynamic allocation
             sizeof(g_canard_memory_pool),      // Size of the above, in bytes
             UAVCAN_onTransferReceived,         // Callback, see CanardOnTransferReception
             UAVCAN_shouldAcceptTransfer,       // Callback, see CanardShouldAcceptTransfer
             NULL);
  canardSetLocalNodeID(&g_canard, EE_Id);
}

void UAVCAN_SendCanard(void)
{
  CanardCANFrame* txf = canardPeekTxQueue(&g_canard);

  while (txf)
  {
    bool tx_res = CANBUS_Send(txf->id, txf->data, txf->data_len);
    if (tx_res == false)
    {
      /**< Fails to send */
      counterErr++;
      break;
    }
    counterTx++;
    canardPopTxQueue(&g_canard);
    txf = canardPeekTxQueue(&g_canard);
  }
}

void UAVCAN_ReceiveCanard(TCanMsg *rx_msg)
{
  CanardCANFrame rx_frame;

  /**< UAVCAN needs Extended packet marking! Bit 31 of CAN ID */
  rx_frame.id = rx_msg->Id | CANARD_CAN_FRAME_EFF;
  rx_frame.data_len = rx_msg->Len;
  memcpy(rx_frame.data, rx_msg->Data, rx_msg->Len);
  counterRx++;
  canardHandleRxFrame(&g_canard, &rx_frame, xTaskGetTickCount() * 1000);

  //int res = CANBUS_Receive(&rx_frame.id, rx_frame.data, &rx_frame.data_len);
  //if (res)
  //{
  //  canardHandleRxFrame(&g_canard, &rx_frame, xTaskGetTickCount() * 1000);
  //}
  //signalTag(rawcommandTransfer);
}

void UAVCAN_MakeNodeStatusMessage(uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE])
{
  uint8_t node_health = UAVCAN_NODE_HEALTH_OK;
  uint8_t node_mode   = UAVCAN_NODE_MODE_OPERATIONAL;
  memset(buffer, 0, UAVCAN_NODE_STATUS_MESSAGE_SIZE);
  uint32_t uptime_sec = (xTaskGetTickCount() / 1000);
  canardEncodeScalar(buffer,  0, 32, &uptime_sec);
  canardEncodeScalar(buffer, 32,  2, &node_health);
  canardEncodeScalar(buffer, 34,  3, &node_mode);
}

void UAVCAN_SpinCanard(void)
{
  uint8_t buffer[UAVCAN_NODE_STATUS_MESSAGE_SIZE];
  static uint8_t transfer_id = 0;                           // This variable MUST BE STATIC; refer to the libcanard documentation for the background

  UAVCAN_MakeNodeStatusMessage(buffer);
  canardBroadcast(&g_canard,
                  UAVCAN_NODE_STATUS_DATA_TYPE_SIGNATURE,
                  UAVCAN_NODE_STATUS_DATA_TYPE_ID,
                  &transfer_id,
                  CANARD_TRANSFER_PRIORITY_LOW,
                  buffer,
                  UAVCAN_NODE_STATUS_MESSAGE_SIZE);                         //some indication
}

void readUniqueID(uint8_t* out_uid)
{
  for (uint8_t i = 0; i < UNIQUE_ID_LENGTH_BYTES; i++)
  {
    out_uid[i] = i;
  }
}

uint16_t UAVCAN_MakeNodeInfoMessage(uint8_t buffer[UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE])
{
  memset(buffer, 0, UAVCAN_GET_NODE_INFO_RESPONSE_MAX_SIZE);
  UAVCAN_MakeNodeStatusMessage(buffer);

  buffer[7] = APP_VERSION_MAJOR;
  buffer[8] = APP_VERSION_MINOR;
  buffer[9] = 1;                          // Optional field flags, VCS commit is set
  uint32_t u32 = GIT_HASH;
  canardEncodeScalar(buffer, 80, 32, &u32);

  readUniqueID(&buffer[24]);
  const size_t name_len = strlen(APP_NODE_NAME);
  memcpy(&buffer[41], APP_NODE_NAME, name_len);
  return 41 + name_len ;
}

void rawcmdHandleCanard(CanardRxTransfer* transfer)
{
//  uint16_t value;
//  int offset= (int)parameters[4].val * 14;
//
//  rawcommandTransfer++;
//  canardDecodeScalar(transfer, offset, 14, true, &value);
//  pwmUpdate(value);
}

void pwmUpdate(uint16_t canValue)
{
  int pwmValue = canValue / 8;
  pwmValue += 1000;
  //__HAL_TIM_SET_COMPARE(&htim3,TIM_CHANNEL_4,pwmVaule);
}

param_t* UAVCAN_GetParamByIndex(uint16_t index)
{
  if (index >= ARRAY_SIZE(parameters))
  {
    return NULL;
  }

  return &parameters[index];
}

param_t* UAVCAN_GetParamByName(uint8_t * name)
{
  for (uint16_t i = 0; i < ARRAY_SIZE(parameters); i++)
  {
    if (strncmp((char const*)name, (char const*)parameters[i].name,strlen((char const*)parameters[i].name)) == 0)
    {
      return &parameters[i];
    }
  }
  return NULL;
}

 uint16_t UAVCAN_EncodeParamCanard(param_t *p, uint8_t *buffer)
{
  uint8_t n     = 0;
  int offset    = 0;
  uint8_t tag   = 1;
  if (p == NULL)
  {
    tag = 0;
    canardEncodeScalar(buffer, offset, 5, &n);
    offset += 5;
    canardEncodeScalar(buffer, offset, 3, &tag);
    offset += 3;

    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset, 2, &tag);
    offset += 2;

    canardEncodeScalar(buffer, offset, 6, &n);
    offset += 6;
    canardEncodeScalar(buffer, offset, 2, &tag);
    offset += 2;
    buffer[offset / 8] = 0;
    return (offset / 8 + 1);
  }
  canardEncodeScalar(buffer, offset, 5, &n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, p->pVal);
  offset += 64;

  canardEncodeScalar(buffer, offset, 5, &n);
  offset += 5;
  canardEncodeScalar(buffer, offset, 3, &tag);
  offset += 3;
  canardEncodeScalar(buffer, offset, 64, &p->defval);
  offset += 64;

  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset, 2, &tag);
  offset += 2;
  canardEncodeScalar(buffer, offset, 64, &p->max);
  offset += 64;

  canardEncodeScalar(buffer, offset, 6, &n);
  offset += 6;
  canardEncodeScalar(buffer, offset, 2, &tag);
  offset += 2;
  canardEncodeScalar(buffer, offset, 64, &p->min);
  offset += 64;

  memcpy(&buffer[offset / 8], p->name, strlen((char const*)p->name));
  return (offset/8 + strlen((char const*)p->name));
}

 void getsetHandleCanard(CanardRxTransfer* transfer)
{
  uint16_t index = 0xFFFF;
  uint8_t tag    = 0;
  int offset     = 0;
  int64_t val    = 0;
  canardDecodeScalar(transfer, offset, 13, false, &index);
  offset += 13;
  canardDecodeScalar(transfer, offset, 3, false, &tag);
  offset += 3;

  if (tag == 1)
  {
    canardDecodeScalar(transfer, offset, 64, false, &val);
    offset += 64;
  }

  uint16_t n = transfer->payload_len - offset / 8 ;
  uint8_t name[16] = "";
  for (int i = 0; i < n; i++)
  {
    canardDecodeScalar(transfer, offset, 8, false, &name[i]);
    offset += 8;
  }

  param_t *p = NULL;

  if (strlen((char const*)name))
  {
    p = UAVCAN_GetParamByName(name);
  } else
  {
    p = UAVCAN_GetParamByIndex(index);
  }

  if ((p) && (tag == 1))
  {
      p->val = val;
  }
  uint8_t  buffer[64] = "";
  uint16_t len = UAVCAN_EncodeParamCanard(p, buffer);
  int result = canardRequestOrRespond(&g_canard,
                                      transfer->source_node_id,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_SIGNATURE,
                                      UAVCAN_PROTOCOL_PARAM_GETSET_ID,
                                      &transfer->transfer_id,
                                      transfer->priority,
                                      CanardResponse,
                                      &buffer[0],
                                      (uint16_t)len);
}
