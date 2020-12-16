#ifndef _RS485CMD_H
#define _RS485CMD_H

//************************************************************************************
//*******************     Actuator's ID commands    **********************************
//************************************************************************************
#define RS485_CMD_SETID         0xAA    // Load Actuator ID command
#define RS485_ANS_SETID         0x55    // response code

#define RS485_CMD_GETID         0xDA    // report Station ID command
#define RS485_ANS_GETID      	  0x6D    // response code

//************************************************************************************
//*******************         Actuator's position commands        ********************
//************************************************************************************
#define RS485_CMD_SETPOS100     0xDD    // servo position command of ±100° area
#define RS485_ANS_SETPOS100     0x44    // response code

#define RS485_CMD_SETPOS170     0x76    // servo position command of ±170° area
#define RS485_ANS_SETPOS170     0x56    // response code

#define C_Set_Pos170_Silent     0x77    // servo new position command without response

#define C_Set_Scaled            0x78    // servo position command of ±170° area
#define R_Set_Scaled            0x58    // response code

#define C_Act_Pos_Scaled        0x79    // report of actual position (±170°) command
#define R_Act_Pos_Scaled        0x59    // response code

#define C_Act_Pos170_report     0x69    // report of actual position (±170°) command
#define R_Act_Pos170_report     0x49    // response code

#define C_Failsafe_Pos          0xBB	// set failsafe position command
#define R_Failsafe_Pos          0x5D	// response code

#define C_Fs_Pos_report         0x90	// report failsafe position command
#define R_Fs_Pos_report         0x60	// response code

#define C_Set_as_Zero           0x99    // Set current Position as zero command
#define R_Set_as_Zero           0x4C    // response code

#define C_Failsafe_Time         0xCC	// set failsafe timeout command
#define R_Failsafe_Time         0x66	// response code

#define C_Fs_Time_report        0x91	// report failsafe timeout command
#define R_Fs_Time_report        0x61	// response code

#define C_Set_as_Failsafe       0xBC    // set current position as new failsafe position command
#define R_Set_as_Failsafe       0x5C    // response code

//************************************************************************************
//*******************   Actuator's control values commands  **************************
//************************************************************************************
#define RS485_CMD_GETCURRENT    0xB0    // Readout of current Consumption command. 1 unit=20mA
#define RS485_ANS_GETCURRENT    0x30    // response code

#define RS485_CMD_GETTEMP       0xC0    // Readout of Temperatures command
#define RS485_ANS_GETTEMP       0x10    // response code

#define RS485_CMD_GETVOLTAGE    0xB1    // Readout of Input Voltages command. 1 unit=200mV
#define RS485_ANS_GETVOLTAGE    0x31    // response code

#define RS485_CMD_GETRUNTIME    0xC2    // C2 ID 00 00 - hours(max.65536H), C2 ID 00 01 - min, sec
#define RS485_ANS_GETRUNTIME    0x12    // response code

#define RS485_CMD_GETMAGNET     0xE5    // Readout of magnet sensor value and control bits
#define RS485_ANS_GETMAGNET     0x45    // response code

//************************************************************************************
//*******************   Other commands  **********************************************
//************************************************************************************

#define Cmd_Motor_Power         0x9A    // Motor power command (power limit if 0x00FF, 0x8000-power in EEPROM, 0x8001-motor brake, 0x8002-motor report)
#define Cmd_Motor_Power_back    0x4A    // Motor power command response

//************************************************************************************
//*******************     EEPROM access commands   ***********************************
//************************************************************************************
#define RS485_CMD_READ_MP       0xE7    // address range 0x20...0x27 e.g.: E7 1F 24 24 - read EEPROM cell 0x24 (Maximal Power)
#define RS485_ANS_READ_MP       0x47    // response code

#define RS485_CMD_WRITE_MP      0xE8    // address range 0x20...0x2F e.g.: E8 1F 24 FF - write 0xFF to EEPROM cell 0x24 (Maximal Power)
#define RS485_ANS_WRITE_MP      0x48    // response code

#define C_Read_Serial_Num       0xE0    // Readout of Electronic Serial Number command
#define R_Read_Serial_Num	      0x40    // response code

#define C_Read_Product_Descr    0xE1    // Readout of Product Description command
#define R_Read_Product_Descr	  0x41    // response code

#define C_Read_Firmware_Num	    0xE2    // Readout of Software Revision Number command
#define R_Read_Firmware_Num	    0x42    // response code

#define C_Read_Hardware_Num	    0xE3    // Readout of Hardware Revision Number command
#define R_Read_Hardware_Num	    0x43    // response code

#define C_Write_Serial_Num      0xD6    // "D6 1F 41 10 - writing letter "A" to EEPROM as #16 in "Electronic Serial Number"
#define R_Write_Serial_Num      0x86    // response code

#define C_Write_Prod_Descr	    0xD7    // "D7 1F 42 11 - writing letter "B" to EEPROM as #17 in "Product descriptor"
#define R_Write_Prod_Descr	    0x87    // response code

#define C_Write_FWrev_num       0xD8    // "D8 1F 43 01 - writing letter "C" to EEPROM as #1 in "Firmware revision"
#define R_Write_FWrev_num       0x88    // response code

#define C_Write_HWrev_num       0xD9    // "D9 1F 44 02 - writing letter "D" to EEPROM as #2 in "Hardware revision"
#define R_Write_HWrev_num       0x89    // response code

#define RS485_CMD_READ_EELOW    0xF8    // read any low 0x00...0xFF (adr in arg2)EEPROM cell (if arg1=0 -read EEPROM, arg1=1 - read FRAM)
#define RS485_ANS_READ_EELOW    0xF8    // response code

#define RS485_CMD_WRITE_EELOW   0xF9    // write any low 0x00...0xFF EEPROM cell
#define RS485_ANS_WRITE_EELOW   0xF9    // response code

#define RS485_CMD_READ_EEHIGH   0xE9    // "E9 1F 80 80 - read EEPROM cell 0x180" (High part of EEPROM 100..1FF)
#define RS485_ANS_READ_EEHIGH   0x49    // response code

#define RS485_CMD_WRITE_EEHIGH  0xFA    // write any high 0x0100...0x01FF EEPROM cell (command needs an access bit)
#define RS485_ANS_WRITE_EEHIGH  0xFA    // response code

#define RS485_CMD_ACCESSBIT     0xFF    // set EEPROM write access bit
#define RS485_ANS_ACCESSBIT     0xFF    // response code

#define RS485_ACCESS_SIGN1      0x41    // signature for EEPROM write access command
#define RS485_ACCESS_SIGN2      0x3F

#define RS485_ACCESS_ANS1       'A'     // answer for EEPROM write access command
#define RS485_ACCESS_ANS2       'E'


#endif
