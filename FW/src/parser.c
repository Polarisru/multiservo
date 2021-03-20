#include "drivers.h"
#include "actions.h"
#include "analog.h"
//#include "bootloader.h"
#include "canbus.h"
#include "conversion.h"
#include "eeprom.h"
#include "fix_fft.h"
#include "global.h"
#include "outputs.h"
#include "parser.h"
#include "rs485.h"
#include "utils.h"
#include "version.h"

#define ADC_SAMPLES   256
uint8_t fft_data[ADC_SAMPLES];
int8_t im[ADC_SAMPLES];


// - ID -
const char parserCmdID[] = "ID";
const char parserHlpID[] = "get device ID";
const char parserOutID[] = "%s";

// - VN -
const char parserCmdVN[] = "VN";
const char parserOutVN[] = "N:%02d.%02d";
const char parserHlpVN[] = "get firmware version";

// - HW -
const char parserCmdHW[] = "HW";
const char parserOutHW[] = "HW:%02d";
const char parserHlpHW[] = "get hardware revision";

// - GB -
const char parserCmdGB[] = "GB";
const char parserHlpGB[] = "get data from current buffer (8 bytes)";
const char parserInGB[]  = "%d";
const char parserOutGB[] = "%s";

// - GCI -
const char parserCmdGCI[] = "GCI";
const char parserHlpGCI[] = "get instant current value, A";
const char parserOutGCI[] = "I:%04.2f";

// - GCP -
const char parserCmdGCP[] = "GCP";
const char parserHlpGCP[] = "get peak current value, A";
const char parserOutGCP[] = "I:%04.1f";

// - GP -
const char parserCmdGP[] = "GP";
const char parserHlpGP[] = "get position, degree";
const char parserOutGP[] = "P:%05.1f";

// - GT -
const char parserCmdGT[] = "GT";
const char parserHlpGT[] = "get module temperature, degree";
const char parserOutGT[] = "T:%.1f";

// - GUS -
const char parserCmdGUS[] = "GUS";
const char parserHlpGUS[] = "get power supply voltage, V";
const char parserOutGUS[] = "US:%04.1f";

// - GU -
const char parserCmdGU[] = "GU";
const char parserHlpGU[] = "get feedback voltage, V";
const char parserOutGU[] = "U:%04.1f";

// - SB -
const char parserCmdSB[] = "SB";
const char parserHlpSB[] = "set baudrate for motor communication";
const char parserInSB[]  = "%d:%d";

// - SID -
const char parserCmdSID[] = "SID";
const char parserHlpSID[] = "set ID for bus protocols";
const char parserInSID[]  = "%d";

// - SI -
const char parserCmdSI[] = "SI";
const char parserHlpSI[] = "set interface type (0 - PWM, 1 - RS485, 2 - CAN, 3 - AM)";
const char parserInSI[]  = "%1d";

// - DF -
const char parserCmdDF[] = "DF";
const char parserHlpDF[] = "do FFT";

// - DP -
const char parserCmdDP[] = "DP";
const char parserHlpDP[] = "move to position, degree";
const char parserInDP[]  = "%f";

// - CC -
const char parserCmdCC[] = "CC";
const char parserHlpCC[] = "calibrate current";

// - CUS -
const char parserCmdCUS[] = "CUS";
const char parserHlpCUS[] = "calibrate power supply voltage measurement";
const char parserInCUS[]  = "%f";

// - RB -
const char parserCmdRB[] = "RB";
const char parserHlpRB[] = "read byte from motor EEPROM at address X (HEX value)";
const char parserInRB[]  = "%X";
const char parserOutRB[] = "B:0x%02X";

// - WB -
const char parserCmdWB[] = "WB";
const char parserHlpWB[] = "write byte to motor EEPROM at address X:Y (HEX values)";
const char parserInWB[]  = "%X:%02X";

// - LED -
const char parserCmdLED[] = "LED";
const char parserHlpLED[] = "switch external bicolor LED on/off (0 – off, 1 – red, 2 - green)";
const char parserInLED[]  = "%1d";

// - ADC -
const char parserCmdADC[] = "ADC";
const char parserHlpADC[] = "get raw ADC value for channel x (ADCx)";
const char parserInADC[]  = "%d";
const char parserOutADC[] = "0x%04X";

// - PWR -
const char parserCmdPWR[] = "PWR";
const char parserHlpPWR[] = "set power to servo (0 - Off, 1 - On)";
const char parserInPWR[]  = "%1d";

// - OUT -
const char parserCmdOUT[] = "OUT";
const char parserHlpOUT[] = "switch output x on/off (0 - Off, 1 - On)";
const char parserInOUT[]  = "%d:%1d";

// - BLS -
const char parserCmdBLS[] = "BLS";
const char parserHlpBLS[] = "BL: start";

// - BLQ -
const char parserCmdBLQ[] = "BLQ";
const char parserHlpBLQ[] = "BL: quit";

// - BLF -
const char parserCmdBLF[] = "BLF";
const char parserHlpBLF[] = "BL: write Flash page";
const char parserInBLF[]  = "%02X";

// - BLC -
const char parserCmdBLC[] = "BLC";
const char parserHlpBLC[] = "BL: check Flash page (BLCxx:yyyy, xx - page number as HEX, yyyy - CRC16 as HEX)";
const char parserInBLC[]  = "%02X:%04X";

// - BLE -
const char parserCmdBLE[] = "BLE";
const char parserHlpBLE[] = "BL: write byte value to EEPROM (BLExxxx:yy, xxxx - address as HEX, yy - value as HEX)";
const char parserInBLE[]  = "%04X:%02X";

// - PSW -
const char parserCmdPSW[] = "PSW";
const char parserHlpPSW[] = "set secured mode";
const char parserInPSW[]  = "%d";

// - PAUSE -
const char parserCmdPAUSE[] = "PAUSE";
const char parserHlpPAUSE[] = "make pause for x msec";
const char parserInPAUSE[]  = "%d";

// - UPD -
const char parserCmdUPD[] = "BL";
const char parserHlpUPD[] = "start updater procedure";
const char parserInUPD[]  = "%d";


const char parserOk[]        = "OK";
const char parserErrCmd[]    = "E.C";
const char parserErrParam[]  = "E.P";
const char parserErrHw[]     = "E.H";
const char parserErrMode[]   = "E.M";
const char parserErrResult[] = "E.R";
const char parserErrBusy[]   = "E.B";
const char parserErrBoot[]   = "E.";
const char parserErrSecured[] = "E.S";

/**< Parser array with commands and formatters */
const TParsedItem PARSER_Items[PARSER_CMD_LAST] =
{
  {parserCmdID,     NULL,            parserOutID,      parserHlpID,     PARSER_SRC_ALL,   false},
  {parserCmdVN,     NULL,            parserOutVN,      parserHlpVN,     PARSER_SRC_ALL,   false},
  {parserCmdHW,     NULL,            parserOutHW,      parserHlpHW,     PARSER_SRC_ALL,   false},

  {parserCmdGB,     parserInGB,      parserOutGB,      parserHlpGB,     PARSER_SRC_ALL,   false},
  {parserCmdGCI,    NULL,            parserOutGCI,     parserHlpGCI,    PARSER_SRC_ALL,   false},
  {parserCmdGCP,    NULL,            parserOutGCP,     parserHlpGCP,    PARSER_SRC_ALL,   false},
  {parserCmdGP,     NULL,            parserOutGP,      parserHlpGP,     PARSER_SRC_ALL,   false},
  {parserCmdGT,     NULL,            parserOutGT,      parserHlpGT,     PARSER_SRC_ALL,   false},
  {parserCmdGUS,    NULL,            parserOutGUS,     parserHlpGUS,    PARSER_SRC_ALL,   false},
  {parserCmdGU,     NULL,            parserOutGU,      parserHlpGU,     PARSER_SRC_ALL,   false},

  {parserCmdSB,     parserInSB,      NULL,             parserHlpSB,     PARSER_SRC_ALL,   false},
  {parserCmdSID,    parserInSID,     NULL,             parserHlpSID,    PARSER_SRC_ALL,   false},
  {parserCmdSI,     parserInSI,      NULL,             parserHlpSI,     PARSER_SRC_ALL,   false},

  {parserCmdDF,     NULL,            NULL,             parserHlpDF,     PARSER_SRC_ALL,   false},
  {parserCmdDP,     parserInDP,      NULL,             parserHlpDP,     PARSER_SRC_ALL,   false},

  {parserCmdCC,     NULL,            NULL,             parserHlpCC,     PARSER_SRC_ALL,   false},
  {parserCmdCUS,    parserInCUS,     NULL,             parserHlpCUS,    PARSER_SRC_ALL,   false},

  {parserCmdRB,     parserInRB,      parserOutRB,      parserHlpRB,     PARSER_SRC_ALL,   false},
  {parserCmdWB,     parserInWB,      NULL,             parserHlpWB,     PARSER_SRC_ALL,   false},

  {parserCmdLED,    parserInLED,     NULL,             parserHlpLED,    PARSER_SRC_ALL,   false},
  {parserCmdADC,    parserInADC,     parserOutADC,     parserHlpADC,    PARSER_SRC_ALL,   true},
  {parserCmdPWR,    parserInPWR,     NULL,             parserHlpPWR,    PARSER_SRC_ALL,   false},
  {parserCmdOUT,    parserInOUT,     NULL,             parserHlpOUT,    PARSER_SRC_ALL,   true},

  {parserCmdBLS,    NULL,            NULL,             parserHlpBLS,    PARSER_SRC_COMM,  false},
  {parserCmdBLQ,    NULL,            NULL,             parserHlpBLQ,    PARSER_SRC_COMM,  false},
  {parserCmdBLF,    parserInBLF,     NULL,             parserHlpBLF,    PARSER_SRC_COMM,  false},
  {parserCmdBLC,    parserInBLC,     NULL,             parserHlpBLC,    PARSER_SRC_COMM,  false},
  {parserCmdBLE,    parserInBLE,     NULL,             parserHlpBLE,    PARSER_SRC_COMM,  false},

  {parserCmdPSW,    parserInPSW,     NULL,             parserHlpPSW,    PARSER_SRC_COMM,  false},

  {parserCmdPAUSE,  parserInPAUSE,   NULL,             parserHlpPAUSE,  PARSER_SRC_TEXT,  false},

  {parserCmdUPD,    parserInUPD,     NULL,             parserHlpUPD,    PARSER_SRC_COMM,  false}
};

/** \brief Search for an index in the string array
 *
 * \param [in] items Command set items to compare
 * \param [in] buffer Buffer with received data
 * \return Index of the command if found or PARSER_CMD_LAST if not
 *
 */
uint8_t PARSER_GetIndex(TParsedItem const *items, char *buffer)
{
  uint8_t i;

  UTILS_StrUpr(buffer);
  i = 0;
  for (i = 0; i < PARSER_CMD_LAST; i++)
  {
    if (0 == strncmp(buffer, items[i].itemStr, strlen(items[i].itemStr)))
      return i;
  }
  return PARSER_CMD_LAST;
}

/** \brief Process command with internal parser
 *
 * \param [in] cmd Data buffer with command to process
 * \param [out] buff Buffer for reply
 * \param [in] source Data source (UART communication or text script)
 * \return Nothing
 *
 */
void PARSER_Process(char *cmd, char *buff, uint8_t source)
{
  char const *errMsg;
  uint8_t cmd_length;
  uint16_t pos;
  uint8_t index;
  uint32_t intVal0, intVal1, intVal2;
  uint8_t bVal;
  float flVal, flVal2;
  uint8_t data[8];
  char str[32];

  /**< Check command and get its index */
  index = PARSER_GetIndex(PARSER_Items, cmd);

  if ((index >= PARSER_CMD_LAST) || ((PARSER_Items[index].source & source) == 0))
  {
    errMsg = parserErrCmd;
  } else
  {
    /**< Command found in the list of commands, process it */
    errMsg = NULL;
    cmd_length = strlen(PARSER_Items[index].itemStr);
    pos = strlen(cmd);
    if (pos > cmd_length)
    {
      cmd = &cmd[cmd_length];
    } else
    {
      cmd = &cmd[pos];
    }
    /**< Process commands */
    switch (index)
    {
      case PARSER_CMD_ID:
        /**< Get ID of the device (SPAN) */
        sprintf(buff, PARSER_Items[index].outFmt, DEVICE_ID);
        break;

      case PARSER_CMD_VN:
        /**< Get firmware version */
        sprintf(buff, PARSER_Items[index].outFmt, DEVICE_VERSION[0], DEVICE_VERSION[1]);
        break;

      case PARSER_CMD_HW:
        /**< Get hardware revision */
        sprintf(buff, PARSER_Items[index].outFmt, VERSION_GetHW());
        break;

      case PARSER_CMD_GB:
        /**< Get data from current buffer (8 bytes) */
        if (sscanf(cmd, PARSER_Items[index].inFmt, &intVal0) != 1)
        {
          errMsg = parserErrParam;
          break;
        }
        if (ANALOG_GetBuff(intVal0, data) == false)
        {
          errMsg = parserErrResult;
          break;
        }
        str[0] = 0;
        for (bVal = 0; bVal < 8; bVal++)
          strcat(str, UTILS_ToHex(data[bVal]));
        sprintf(buff, PARSER_Items[index].outFmt, str);
        break;

      case PARSER_CMD_GCI:
        /**< Get instant current value */
        sprintf(buff, PARSER_Items[index].outFmt, CONVERSION_GetCurrent(ADC_TYPE_CURRENT));
        break;

      case PARSER_CMD_GCP:
        /**< Get measured peak current value */
        sprintf(buff, PARSER_Items[index].outFmt, GLOBAL_PeakCurrent);
        break;

      case PARSER_CMD_GP:
        /**< Get position from SSI */
        ACTIONS_GetPosition(&flVal);
        sprintf(buff, PARSER_Items[index].outFmt, flVal);
        break;

      case PARSER_CMD_GT:
        /**< Get temperature */
        flVal = TEMP_GetValue();
        /**< If sensor is not connected */
        if (flVal < 0)
        {
          errMsg = parserErrHw;
          break;
        }
        sprintf(buff, PARSER_Items[index].outFmt, flVal);
        break;

      case PARSER_CMD_GUS:
        /**< Get power supply voltage */
        sprintf(buff, PARSER_Items[index].outFmt, CONVERSION_GetSupplyVoltage());
        break;

      case PARSER_CMD_GU:
        /**< Get feedback voltage */
        sprintf(buff, PARSER_Items[index].outFmt, CONVERSION_GetVoltage());
        break;

      case PARSER_CMD_SB:
        /**< Set baudrate for motor communication */
        intVal2 = sscanf(cmd, PARSER_Items[index].inFmt, &intVal0, &intVal1);
        if ((intVal2 < 1) || (intVal2 > 2))
        {
          errMsg = parserErrParam;
          break;
        }
        switch (GLOBAL_ConnMode)
        {
          case CONN_MODE_PWM:
            errMsg = parserErrMode;
            break;
          case CONN_MODE_RS485:
            RS485_SetBaudrate(intVal0);
            errMsg = parserOk;
            break;
          case CONN_MODE_CAN:
            if (intVal2 > 1)
            {
              errMsg = parserErrParam;
              break;
            }
            intVal1 = intVal0;
            if (CANBUS_SetBaudrate(intVal0, intVal1) == true)
              errMsg = parserOk;
            else
              errMsg = parserErrParam;
            break;
          case CONN_MODE_AMAZON:
            if (intVal2 == 1)
            {
              /**< Without baudrate switching */
              intVal1 = intVal0;
            }
            if (CANBUS_SetBaudrate(intVal0, intVal1) == true)
              errMsg = parserOk;
            else
              errMsg = parserErrParam;
            break;
        }
        break;

      case PARSER_CMD_SID:
        /**< Set ID for bus protocols */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (ACTIONS_SetId((uint8_t)intVal0) == true)
          errMsg = parserOk;
        else
          errMsg = parserErrParam;
        break;

      case PARSER_CMD_SI:
        /**< Set interface (PWM/RS485/CAN/UAVCAN/FUTABA) */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (ACTIONS_SetMode((uint8_t)intVal0) == false)
        {
          errMsg = parserErrParam;
          break;
        }
        errMsg = parserOk;
        break;

      case PARSER_CMD_DF:
        ANALOG_FillFftBuff(fft_data, ADC_SAMPLES);
        memset(im, 0, sizeof(im));
        fix_fft(fft_data, im, 8, false);
        errMsg = parserOk;
        break;

      case PARSER_CMD_DP:
        /**< Move servo to position */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &flVal))
        {
          errMsg = parserErrParam;
          break;
        }
        if (ACTIONS_MoveToPosition(flVal) == false)
        {
          errMsg = parserErrHw;
          break;
        }
        errMsg = parserOk;
        break;

      case PARSER_CMD_CC:
        /**< Calibrate current (get offset) */
        intVal0 = ANALOG_GetValueADC1(ADC_CHANNEL_I, ADC_TYPE_CURRENT);
        EE_CurrOffset = (uint16_t)intVal0;
        EEPROM_SaveVariable(&EE_CurrOffset);
        EEPROM_SaveBoth();
        errMsg = parserOk;
        break;

      case PARSER_CMD_CUS:
        /**< Calibrate power supply voltage */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &flVal))
        {
          errMsg = parserErrParam;
          break;
        }
        if (flVal < 0.1f)
        {
          errMsg = parserErrParam;
          break;
        }
        flVal2 = flVal / CONVERSION_GetSupplyVoltage();
        EE_VoltSupplyDiv *= flVal2;
        EEPROM_SaveVariable(&EE_VoltSupplyDiv);
        EEPROM_SaveBoth();
        errMsg = parserOk;
        break;

      case PARSER_CMD_RB:
        /**< Read byte from servo EEPROM */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (ACTIONS_ReadByte((uint16_t)intVal0, &bVal) == false)
        {
          errMsg = parserErrHw;
          break;
        }
        sprintf(buff, PARSER_Items[index].outFmt, bVal);
        break;

      case PARSER_CMD_WB:
        /**< Write byte to servo EEPROM */
        if (2 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0, &intVal1))
        {
          errMsg = parserErrParam;
          break;
        }
        if (ACTIONS_WriteByte((uint16_t)intVal0, (uint8_t)intVal1) == false)
        {
          errMsg = parserErrHw;
          break;
        }
        errMsg = parserOk;
        break;

      case PARSER_CMD_LED:
        /**< Switch on/off external LED */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        OUTPUTS_Switch(OUTPUTS_LED1, OUTPUTS_SWITCH_OFF);
        OUTPUTS_Switch(OUTPUTS_LED2, OUTPUTS_SWITCH_OFF);
        break;

      case PARSER_CMD_ADC:
        /**< Get raw ADC values */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 >= ADC_CHANNEL_NUM)
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 == ADC_CHANNEL_I)
          sprintf(buff, PARSER_Items[index].outFmt, ANALOG_GetValueADC1((uint8_t)intVal0, ADC_TYPE_CURRENT));
        else
          sprintf(buff, PARSER_Items[index].outFmt, ANALOG_GetValueADC0((uint8_t)intVal0, ADC_TYPE_CURRENT));
        break;

      case PARSER_CMD_PWR:
        /**< Set power (0 - On, 1 - Off) */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 == 0)
        {
          OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_OFF);
          GLOBAL_PowerOn = false;
        } else
        {
          OUTPUTS_Switch(OUTPUTS_SERVO, OUTPUTS_SWITCH_ON);
          GLOBAL_PowerOn = true;
        }
        errMsg = parserOk;
        break;

      case PARSER_CMD_OUT:
        if (2 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0, &intVal1))
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 >= OUTPUTS_LAST)
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal1 == 0)
          OUTPUTS_Switch((uint8_t)intVal0, OUTPUTS_SWITCH_OFF);
        else
          OUTPUTS_Switch((uint8_t)intVal0, OUTPUTS_SWITCH_ON);
        errMsg = parserOk;
        break;

      case PARSER_CMD_BLS:
        /**< Start bootloader */
//        if (BOOTLOADER_Start() == true)
//          errMsg = parserOk;
//        else
//          errMsg = parserErrHw;
        break;

      case PARSER_CMD_BLQ:
        /**< Quit bootloader */
        //BOOTLOADER_Stop();
        errMsg = parserOk;
        break;

      case PARSER_CMD_BLF:
        /**< Write page to flash */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
//        if (BOOTLOADER_WriteFlash((uint8_t)intVal0, false) == true)
//          errMsg = parserOk;
//        else
//          errMsg = parserErrHw;
        break;

      case PARSER_CMD_BLC:
        /**< Check flash page */
        if (2 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0, &intVal1))
        {
          errMsg = parserErrParam;
          break;
        }
//        if (BOOTLOADER_CheckCRC((uint8_t)intVal0, (uint16_t)intVal1) == true)
//          errMsg = parserOk;
//        else
//          errMsg = parserErrHw;
        break;

      case PARSER_CMD_BLE:
        /**< Write value to EEPROM */
        if (2 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0, &intVal1))
        {
          errMsg = parserErrParam;
          break;
        }
//        if (BOOTLOADER_WriteEEPROM((uint16_t)intVal0, (uint8_t)intVal1) == true)
//          errMsg = parserOk;
//        else
//          errMsg = parserErrHw;
        break;

      case PARSER_CMD_PSW:
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 == PASSWORD_SECURED)
        {
          //COMM_SetSecured(true);
          errMsg = parserOk;
        } else
        {
          errMsg = parserErrSecured;
        }
        break;

      case PARSER_CMD_PAUSE:
        /**< Do pause */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        vTaskDelay(intVal0);
        errMsg = parserOk;
        break;

      case PARSER_CMD_UPD:
        /**< Jump to updater (reset MCU) */
        if (1 != sscanf(cmd, PARSER_Items[index].inFmt, &intVal0))
        {
          errMsg = parserErrParam;
          break;
        }
        if (intVal0 != UPD_PASSWORD)
        {
          errMsg = parserErrParam;
          break;
        }
        WDT_Init();
        WDT_Enable();
        while(1);
        break;
    }
  }

  if (errMsg)
  {
    /**< Display error message */
    strcpy(buff, errMsg);
    strcat(buff, LINE_CRLF);
  } else
  {
    /**< Display text buffer */
    strcat(buff, LINE_CRLF);
  }
}
