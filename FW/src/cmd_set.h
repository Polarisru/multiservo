#ifndef CMD_SET_H
#define CMD_SET_H

//************************************************************************************
//*******************     Actuator's ID commands    **********************************
//************************************************************************************
#define CMD_SET_ID              0xAA    // Load Actuator ID command
#define RSP_SET_ID              0x55    // response code

#define CMD_GET_ID              0xDA    // report Station ID command
#define RSP_GET_ID           	  0x6D    // response code

//************************************************************************************
//*******************         Actuator's position commands        ********************
//************************************************************************************
#define CMD_SET_POS_100         0xDD    // servo position command of ±100° area
#define RSP_SET_POS_100         0x44    // response code

#define CMD_SET_POS_170         0x76    // servo position command of ±170° area
#define RSP_SET_POS_170         0x56    // response code

#define CMD_SET_POS_170_SILENT  0x77    // servo new position command without response

#define CMD_SET_SCALED          0x78    // servo position command of ±170° area
#define RSP_SET_SCALED          0x58    // response code

#define CMD_GET_ACT_POS_SCALED  0x79    // report of actual position (±170°) command
#define RSP_GET_ACT_POS_SCALED  0x59    // response code

#define CMD_GET_ACT_POS_170     0x69    // report of actual position (±170°) command
#define RSP_GET_ACT_POS_170     0x49    // response code

#define CMD_SET_FS_POS          0xBB	// set failsafe position command
#define RSP_SET_FS_POS          0x5D	// response code

#define CMD_GET_FS_POS          0x90	// report failsafe position command
#define RSP_GET_FS_POS          0x60	// response code

#define CMD_SET_AS_ZERO         0x99    // Set current Position as zero command
#define RSP_SET_AS_ZERO         0x4C    // response code

#define CMD_SET_FS_TIME         0xCC	// set failsafe timeout command
#define RSP_SET_FS_TIME         0x66	// response code

#define CMD_GET_FS_TIME         0x91	// report failsafe timeout command
#define RSP_GET_FS_TIME         0x61	// response code

#define CMD_SET_AS_FAILSAFE     0xBC    // set current position as new failsafe position command
#define RSP_SET_AS_FAILSAFE     0x5C    // response code

//************************************************************************************
//*******************   Actuator's control values commands  **************************
//************************************************************************************
#define CMD_CURRENT             0xB0    // Readout of current Consumption command. 1 unit=20mA
#define RSP_CURRENT	  	        0x30    // response code

#define CMD_TEMPERATURE    	    0xC0    // Readout of Temperatures command
#define RSP_TEMPERATURE	        0x10    // response code

#define CMD_VOLTAGE             0xB1    // Readout of Input Voltages command. 1 unit=200mV
#define RSP_VOLTAGE	            0x31    // response code

#define CMD_RUNTIMER            0xC2    // C2 ID 00 00 - hours(max.65536H), C2 ID 00 01 - min, sec
#define RSP_RUNTIMER            0x12    // response code

#define CMD_MAGNET	            0xE5    // Readout of magnet sensor value and control bits
#define RSP_MAGNET	            0x45    // response code

//************************************************************************************
//*******************   Other commands  **********************************************
//************************************************************************************

#define CMD_SET_MOTOR_POWER     0x9A    // Motor power command (power limit if 0x00FF, 0x8000-power in EEPROM, 0x8001-motor brake, 0x8002-motor report)
#define RSP_SET_MOTOR_POWER     0x4A    // Motor power command response

//************************************************************************************
//*******************     EEPROM access commands   ***********************************
//************************************************************************************
#define CMD_READ_MP             0xE7    // address range 0x20...0x27 e.g.: E7 1F 24 24 - read EEPROM cell 0x24 (Maximal Power)
#define RSP_READ_MP             0x47    // response code

#define CMD_WRITE_MP            0xE8    // address range 0x20...0x2F e.g.: E8 1F 24 FF - write 0xFF to EEPROM cell 0x24 (Maximal Power)
#define RSP_WRITE_MP		        0x48    // response code

#define CMD_READ_SERIAL_NUM     0xE0    // Readout of Electronic Serial Number command
#define RSP_READ_SERIAL_NUM	    0x40    // response code

#define CMD_READ_PROD_DESCR     0xE1    // Readout of Product Description command
#define RSP_READ_PROD_DESCR	    0x41    // response code

#define CMD_READ_FIRMWARE_NUM   0xE2    // Readout of Software Revision Number command
#define RSP_READ_FIRMWARE_NUM   0x42    // response code

#define CMD_READ_HWREV_NUM	    0xE3    // Readout of Hardware Revision Number command
#define RSP_READ_HWREV_NUM	    0x43    // response code

#define CMD_WRITE_SERIAL_NUM    0xD6    // "D6 1F 41 10 - writing letter "A" to EEPROM as #16 in "Electronic Serial Number"
#define RSP_WRITE_SERIAL_NUM    0x86    // response code

#define CMD_WRITE_PROD_DESCR	  0xD7    // "D7 1F 42 11 - writing letter "B" to EEPROM as #17 in "Product descriptor"
#define RSP_WRITE_PROD_DESCR	  0x87    // response code

#define CMD_WRITE_FIRMWARE_NUM  0xD8    // "D8 1F 43 01 - writing letter "C" to EEPROM as #1 in "Firmware revision"
#define RSP_WRITE_FIRMWARE_NUM  0x88    // response code

#define CMD_WRITE_HWREV_NUM     0xD9    // "D9 1F 44 02 - writing letter "D" to EEPROM as #2 in "Hardware revision"
#define RSP_WRITE_HWREV_NUM     0x89    // response code

#define CMD_RD_EEPROM_LOW       0xF8    // read any low 0x00...0xFF (adr in arg2)EEPROM cell (if arg1=0 -read EEPROM, arg1=1 - read FRAM)
#define RSP_RD_EEPROM_LOW       0xF8    // response code

#define CMD_WR_EEPROM_LOW       0xF9    // write any low 0x00...0xFF EEPROM cell
#define RSP_WR_EEPROM_LOW       0xF9    // response code

#define CMD_RD_EEPROM_HIGH      0xE9    // "E9 1F 80 80 - read EEPROM cell 0x180" (High part of EEPROM 100..1FF)
#define RSP_RD_EEPROM_HIGH      0x49    // response code

#define CMD_WR_EEPROM_HIGH      0xFA    // write any high 0x0100...0x01FF EEPROM cell (command needs an access bit)
#define RSP_WR_EEPROM_HIGH      0xFA    // response code

#endif
