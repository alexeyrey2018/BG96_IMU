// Define the address of database

// Block code for loading the program
#define EEPROM_CONFIG		0x08080000
#define DB_BLOCK_CODE		0x0
// binary for Sensor enable option, 1: enable, 0: disable, SPI1, SPI2, SPI3, ADC1, ADC2, ADC3, GPIO1
#define DB_SENSOR_OPTION	0x1
#define DB_SENSOR_INTERVAL	0x2
#define DB_NETWORK_INTERVAL	0x3

// Device ID is uint32 value (4 ~ 7)
#define DB_DEVICE_ID		0x4

// =============== Sensor Data =====================

// Battery:
// start:	1 byte
// end:		1 byte
// value:	2 byte
// year:	2 byte
// month:	1 byte
// date:	1 byte
// hour:	1 byte
// minute:	1 byte
// second:	1 byte
// status:	1 byte
// 10 bytes x 8 = 80 bytes + 2 bytes
#define DB_BATTERY_START	0x8
#define DB_BATTERY_END		0x9
#define DB_BATTERY_VALUE	0xA
#define DB_BATTERY_UNIT		10

// Temperature
// start:	1 byte
// end:		1 byte
// value:	2 byte
// year:	2 byte
// month:	1 byte
// date:	1 byte
// hour:	1 byte
// minute:	1 byte
// second:	1 byte
// status:	1 byte
// 10 bytes x 8 = 80 bytes + 2 bytes
#define DB_TEMP_START	0x5A
#define DB_TEMP_END		0x5B
#define DB_TEMP_VALUE	0x5C
#define DB_TEMP_UNIT	10

// Vibration
// start:	1 byte
// end:		1 byte
// Xvalue:	2 byte
// Yvalue:	2 byte
// Zvalue:	2 byte
// year:	2 byte
// month:	1 byte
// date:	1 byte
// hour:	1 byte
// minute:	1 byte
// second:	1 byte
// status:	1 byte
// 14 bytes x 8 = 112 bytes + 2 bytes
#define DB_VIBRATION_START	0x10C
#define DB_VIBRATION_END	0x10D
#define DB_VIBRATION_VALUE	0x10E
#define DB_VIBRATION_UNIT	14

// Pressure
// start:	1 byte
// end:		1 byte
// value:	2 byte
// year:	2 byte
// month:	1 byte
// date:	1 byte
// hour:	1 byte
// minute:	1 byte
// second:	1 byte
// status:	1 byte
// 10 bytes x 8 = 80 bytes + 2 bytes
#define DB_PRESS_START	0x17E
#define DB_PRESS_END	0x17F
#define DB_PRESS_VALUE	0x180
#define DB_PRESS_UNIT	10

