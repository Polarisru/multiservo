#include "global.h"

/**< List of global variables */
uint32_t GLOBAL_Baudrate;
uint8_t  GLOBAL_ConnMode;
float GLOBAL_PeakCurrent;

/**< EEPROM variables with default values */
uint8_t  EE_Id          = 0x01;     // Id of the servo
uint16_t EE_CanId       = 0x3f0;    // CAN StdId for bootloader
uint8_t  EE_Kd          = 30;       // Differential coefficient for PD-regulator
uint8_t  EE_Kp          = 60;       // Proportional coefficient for PD-regulator
uint8_t  EE_PWM_Min     = 40;       // minimal value of motor PWM
uint8_t  EE_Ko          = 0;        // Offset for motor PWM
uint8_t  EE_PWM_Max     = 255;      // maximal value of motor PWM
uint8_t  EE_Signal_DB   = 1;        // Signal Input deadband
uint16_t EE_Sensor_DB   = 10;        // Magnet sensor deadband
uint16_t EE_Min_us      = 0x0030;   // Min us - input PWM limit
uint16_t EE_Max_us      = 0x0FD0;   // Max us - input PWM limit
uint8_t  EE_Options     = OPTIONS_RESET; // Reverse-Bit0, Failsafe-Bit1
uint8_t  EE_Expansion   = 0x64;     // Travelling Angle Expansion
uint8_t  EE_PWM_Safe    = 128;      // Motorsaver PWM
uint16_t EE_FailsafePos = 0x1770;   // Failsafe position
uint8_t  EE_FailsafeTime = 20;      // Failsafe timeout, 0.1sec
int16_t  EE_Neutral     = 0;        // Neutral position
uint8_t  EE_Saver_Time  = 20;       // Time to enter to saver mode, value in seconds*10
uint8_t  EE_Saver_Frame = 0x32;     // Position to escape from saver mode
uint8_t  EE_CCW_Scaling = 100;      // Scaling factor for CCW rotation
uint8_t  EE_CW_Scaling  = 100;      // Scaling factor for CW rotation
uint16_t EE_Point_cutoff = 4000;    // PWM for carburettor in close state  (dflt 4000 * 0.25=1000 uS)
uint16_t EE_Point_idle  = 4000;     // PWM for carburettor in Idle  state  (dflt 4720 * 0.25=1180 uS)
uint16_t EE_Point_full  = 8000;     // PWM for carburettor in open  state  (dflt 8000 * 0.25=2000 uS)
uint16_t EE_Mag_idle    = 1500;     // magnet position for carburettor in Idle state
uint16_t EE_Mag_full    = 2500;     // magnet position @ 2000uS
uint16_t EE_CO_FB_Voltage = 0;      // Feedback voltage at cut-off position
uint16_t EE_FT_FB_Voltage = 0xfff;  // Feedback voltage at full-throttle position
char     EE_Name[SERVO_NAME_LEN];
int16_t  EE_CurrOffset;
