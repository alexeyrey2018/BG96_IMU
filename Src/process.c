#include "process.h"
#include "usbd_cdc_if.h"
#include "database.h"
#include "device_init.h"
#include "usb_device.h"

extern UART_HandleTypeDef hlpuart1;
extern RTC_HandleTypeDef hrtc;
extern I2C_HandleTypeDef hi2c2;
extern ADC_HandleTypeDef hadc;
ADC_STATE adcState = BATTERY_STATE;
uint16_t batteryLevel = 0;
uint16_t adcValue1 = 0, adcValue2 = 0, adcValue3 = 0;

uint8_t n = 0;
uint8_t blinkMessage[510] = "";

uint8_t receiveData[1000];
uint8_t receivedMessage[200];
uint16_t receiveIndexEnd = 0;
uint16_t receiveIndexStart = 0, flag = 0;
uint16_t receivedLen = 0;

uint16_t start = 0, end = 0;

// void RebootProcess();
void SensorMeasureProcess();
void IoTCommunicationProcess();
void BootBG96();

extern uint8_t SensorMeasurementCase;
extern uint8_t IoTCommunicationCase;


RTC_AlarmTypeDef sAlarm;
RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

/*
void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	SensorMeasurementCase = 1;

	HAL_Delay(1000);
    n = sprintf((char*)&blinkMessage[0], "RTC interrupt for measurement happened.\r\n");
    CDC_Transmit_FS(blinkMessage, n);
}

void HAL_RTCEx_AlarmBEventCallback(RTC_HandleTypeDef *hrtc)
{
	IoTCommunicationCase = 1;

	HAL_Delay(1000);
    n = sprintf((char*)&blinkMessage[0], "RTC interrupt for communication happened.\r\n");
    CDC_Transmit_FS(blinkMessage, n);
}
*/

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
    if (__HAL_ADC_GET_FLAG(hadc, ADC_FLAG_EOC))
    {
        uint16_t temp = HAL_ADC_GetValue(hadc);

        if (adcState == BATTERY_STATE)
        {
            batteryLevel = temp;
        }
        else if (adcState == ASENSE_ADC1)
        {
			adcValue1 = temp;
        }
        else if (adcState == ASENSE_ADC2)
        {
			adcValue2 = temp;
        }
        else if (adcState == ASENSE_ADC3)
        {
			adcValue3 = temp;
        }
    }
}

uint8_t spiSendData[2], spiReceiveData[2];

void SensorModule()
{
    MX_ADC_Init();
    ADC_ChannelConfTypeDef sConfig;

	HAL_ADC_DeInit(&hadc);

}

// Process when measuring sensors
void SensorMeasureProcess()
{
	uint32_t address;
	uint8_t option = 0;
	uint8_t position = 0;

	// RTC setting for sensor measurement process by RTC Alarm A

	// Read the current time and date
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);

	// Star the sensor measurement process
    n = sprintf((char*)&blinkMessage[0], "==== Start sensor measurement process ====\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	//SensorModule();

    // Enable SPI1
    n = sprintf((char*)&blinkMessage[0], "=>Enable SPI1.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

    MX_ADC_Init();
    MX_I2C2_Init();


    // Update next wake time in RTC Alarm A
    n = sprintf((char*)&blinkMessage[0], "=>Update next wake time in RTC Alarm A.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	uint8_t alarmHour = 0;
	uint32_t interval = 0;

	// Read the interval of sensor measurement
	address = EEPROM_CONFIG;
	HAL_FLASHEx_DATAEEPROM_Unlock();  //Unprotect the EEPROM to allow writing
	interval = *((uint32_t *) address);
    HAL_FLASHEx_DATAEEPROM_Lock();  // Reprotect the EEPROM;

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);

	alarmHour = sTime.Minutes + (uint8_t)((interval >> 16) & 0xFF);
	// if alarmHour is larger than 23, then set to 0 and increase the weekday
	if (alarmHour > 59)
	{
		alarmHour -= 60;
		if (sTime.Hours == 23)
		{
			sAlarm.AlarmTime.Hours = 0;
			sAlarm.AlarmDateWeekDay = sDate.WeekDay + 1;
			if (sAlarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY)
				sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
		else
			sAlarm.AlarmTime.Hours = sTime.Hours + 1;
	}
	else
	{
		sAlarm.AlarmTime.Hours = sTime.Hours;
		sAlarm.AlarmDateWeekDay = sDate.WeekDay;
	}

	// set the alarm time
	sAlarm.AlarmTime.Minutes = alarmHour;
	sAlarm.AlarmTime.Seconds = sTime.Seconds;
	sAlarm.AlarmTime.SubSeconds = sTime.SubSeconds;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_PM;
	sAlarm.Alarm = RTC_ALARM_A;



	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);
	n = sprintf((char*)&blinkMessage[0], "=>Measurement:  %d Time:%d:%d   alarm:%d:%d.\r\n", sDate.WeekDay, sTime.Hours, sTime.Minutes, sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
    CDC_Transmit_FS(blinkMessage, n);
	end ++;
	if (end == 9)
		end = 0;

    n = sprintf((char*)&blinkMessage[0], "===== Complete sensor measurement Process ======\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(500);
	__HAL_RCC_PWR_CLK_ENABLE(); 		// Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); 	// Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); 		// Fast wake-up for ultra low power mode
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
	/*
	HAL_PWR_EnterSLEEPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	*/

	HAL_PWR_EnterSTANDBYMode();
}

int connectNetwork();


RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

// Process of communication with server
void IoTCommunicationProcess()
{

	// Star the sensor measurement process
    n = sprintf((char*)&blinkMessage[0], "==== Start Communication process ====\r\n");
    CDC_Transmit_FS(blinkMessage, n);



	HAL_Delay(500);

    // Enable Radio Comm Peripheral
    n = sprintf((char*)&blinkMessage[0], "=>Enabling Radio Communication Peripheral...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	MX_LPUART1_UART_Init();

	// SensorModule();
	end = 1;

    // Boot Radio
	BootBG96();

	HAL_Delay(7000);
    // Register network
	if (connectNetwork() == 1)
	{
		// Download Configuration Update
		n = sprintf((char*)&blinkMessage[0], "=>Sending sensor data\r\n");
		CDC_Transmit_FS(blinkMessage, n);

		// Download Configuration Update
		n = sprintf((char*)&blinkMessage[0], "=>Downloading configuration data\r\n");
		CDC_Transmit_FS(blinkMessage, n);

		receiveConfigurationData();
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "=>Failed to connect to the network\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}

	/*
    // If Program Update == True, Change Blockcode
    n = sprintf((char*)&blinkMessage[0], "=>program updating, but not yet implemented.\n");
    CDC_Transmit_FS(blinkMessage, n);
	*/


    // Update RTC with Network Time
    n = sprintf((char*)&blinkMessage[0], "=>Uploading RTC with Network time.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	// This is dummy value for configuration data from the server.
	// char configStr[50] = "SP 31 2018/12/13_6:01 54 S_1 C_2 EP";

	uint32_t deviceID = 0;
	uint32_t address;

    // Powering Down BG96
	ShutdownBG96();

    // Disable Radio Comm Peripheral
    n = sprintf((char*)&blinkMessage[0], "=>Disabling Radio Communication Peripheral...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_UART_DeInit(&hlpuart1);

    // Update time measurement and communication in RTC Alarm
    n = sprintf((char*)&blinkMessage[0], "=>Update next wake time in RTC Alarm A.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	RTC_AlarmTypeDef sAlarm;// Add the interval to the current time
	uint8_t alarmHour = 0;
	uint32_t interval = 0;

	// Read the current time and date
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD);


	n = sprintf((char*)&blinkMessage[0], "=>Current time2:%d/%d/%d %d %d:%d:%d.\r\n", sDate.Year, sDate.Month, sDate.Date, sDate.WeekDay, sTime.Hours, sTime.Minutes, sTime.Seconds);
    CDC_Transmit_FS(blinkMessage, n);
	// Read the interval of sensor measurement
	//address = DB_NETWORK_INTERVAL + 0x08080000;
	address = EEPROM_CONFIG;
	HAL_FLASHEx_DATAEEPROM_Unlock();  //Unprotect the EEPROM to allow writing
	interval = *((uint32_t *) address);
    HAL_FLASHEx_DATAEEPROM_Lock();  // Reprotect the EEPROM

	alarmHour = sTime.Minutes + (uint8_t)((interval >> 16) & 0xFF);
	// if alarmHour is larger than 23, then set to 0 and increase the weekday
	if (alarmHour > 59)
	{
		alarmHour -= 60;
		if (sTime.Hours == 23)
		{
			sAlarm.AlarmTime.Hours = 0;
			sAlarm.AlarmDateWeekDay = sDate.WeekDay + 1;
			if (sAlarm.AlarmDateWeekDay > RTC_WEEKDAY_SUNDAY)
				sAlarm.AlarmDateWeekDay = RTC_WEEKDAY_MONDAY;
		}
		else
			sAlarm.AlarmTime.Hours = sTime.Hours + 1;
	}
	else
	{
		sAlarm.AlarmTime.Hours = sTime.Hours;
		sAlarm.AlarmDateWeekDay = sDate.WeekDay;
	}

	// set the alarm time
	sAlarm.AlarmTime.Minutes = alarmHour;
	sAlarm.AlarmTime.Seconds = sTime.Seconds;
	sAlarm.AlarmTime.SubSeconds = sTime.SubSeconds;
	sAlarm.AlarmTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
	sAlarm.AlarmTime.StoreOperation = RTC_STOREOPERATION_RESET;
	sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
	sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDMASK_ALL;
	sAlarm.AlarmDateWeekDaySel = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
	sAlarm.AlarmTime.TimeFormat = RTC_HOURFORMAT12_PM;
	sAlarm.Alarm = RTC_ALARM_A;

	HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD);

	n = sprintf((char*)&blinkMessage[0], "=>Measurement: %d Time:%d:%d   alarm:%d:%d.\r\n", sDate.WeekDay, sTime.Hours, sTime.Minutes, sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes);
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(500);

    n = sprintf((char*)&blinkMessage[0], "==== Complete IoT communication Process ===\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_Delay(1000);

	//__HAL_RCC_PWR_CLK_ENABLE(); // Enable Power Control clock
	HAL_PWREx_EnableUltraLowPower(); // Ultra low power mode
	HAL_PWREx_EnableFastWakeUp(); // Fast wake-up for ultra low power mode
	__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
		//HAL_PWR_EnterSLEEPMode( PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

	//HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
	HAL_PWR_EnterSTANDBYMode();

}


void BootBG96()
{
	// Boot Radio
    n = sprintf((char*)&blinkMessage[0], "=>Booting BG96...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_GPIO_WritePin(UC_RAD_PWRKEY_GPIO_Port, UC_RAD_PWRKEY_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(UC_RAD_PWRKEY_GPIO_Port, UC_RAD_PWRKEY_Pin, GPIO_PIN_RESET);
	HAL_Delay(5000);

	// If the status pin is high, turn on the green led
	if (HAL_GPIO_ReadPin(RAD_UC_STATUS_GPIO_Port, RAD_UC_STATUS_Pin) == GPIO_PIN_SET)
	{
    	HAL_GPIO_WritePin(UC_BLUE_LED_GPIO_Port, UC_BLUE_LED_Pin, GPIO_PIN_SET);
		n = sprintf((char*)&blinkMessage[0], "=>BG96 is booted.\n");
		CDC_Transmit_FS(blinkMessage, n);
	}

	// Reset the BG96
	HAL_GPIO_WritePin(UC_RAD_RSTN_GPIO_Port, UC_RAD_RSTN_Pin, GPIO_PIN_SET);
	HAL_Delay(300);
	HAL_GPIO_WritePin(UC_RAD_RSTN_GPIO_Port, UC_RAD_RSTN_Pin, GPIO_PIN_RESET);
}

void ShutdownBG96()
{
    n = sprintf((char*)&blinkMessage[0], "=>Powering Down BG96...\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	HAL_GPIO_WritePin(UC_BLUE_LED_GPIO_Port, UC_BLUE_LED_Pin, GPIO_PIN_RESET);
	HAL_Delay(500);
	n = sprintf((char*)&blinkMessage[0], "AT+QPOWD\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(2000);
}


int connectNetwork()
{
    n = sprintf((char*)&blinkMessage[0], "=>Connect to network.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	// Prevent echo
	n = sprintf((char*)&blinkMessage[0], "ATE0\r\n");
	__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_NEF|UART_CLEAR_OREF);
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(2000);

	 CDC_Transmit_FS(receivedMessage, 20);

	// "ATI" command
	n = sprintf((char*)&blinkMessage[0], "ATI\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);

	HAL_Delay(2000);

	 CDC_Transmit_FS(receivedMessage, 20);
	if (strstr((char*)receivedMessage, "BG96") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "BG96 detected!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "BG96 not detected!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// Check SIM card
	n = sprintf((char*)&blinkMessage[0], "AT+CPIN?\r\n");
	__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_NEF|UART_CLEAR_OREF);
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	 CDC_Transmit_FS(receivedMessage, 20);

	if (strstr((char*)receivedMessage, "READY") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "SIM card Ready!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "SIM card Not Ready!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// Set full functionality
	n = sprintf((char*)&blinkMessage[0], "AT+CFUN=1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set full functionality!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	/*
	// Set full functionality
	n = sprintf((char*)&blinkMessage[0], "AT+COPS?\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(5000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "Verizon") != NULL || strstr((char*)receivedMessage, "MLTNetwork") != NULL )
	{
		n = sprintf((char*)&blinkMessage[0], "Found Operator!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Not Found Operator!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}
	*/

	// Set IoT mode
	n = sprintf((char*)&blinkMessage[0], "AT+QCFG=\"iotopmode\",0,1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set IoT mode!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Set IoT mode Failed!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	return 1;
}


char dateP[30], sensorSelection[30], threshold[30], sensorCycle[20], comCycle[20];
uint32_t selectByte;
uint32_t period;

char latitude[30], longitude[30];
int getGPSData()
{
    char *p;
    
	// Turn on GNSS
	n = sprintf((char*)&blinkMessage[0], "AT+QGPS=1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Turn on GPS!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}
	
	//After turning on GNSS, NMEA sentences will be outputted from "usbnmea" port by default
	n = sprintf((char*)&blinkMessage[0], "AT+QGPSLOC=2\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "+QGPSLOC:") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Read GPS Data!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
        
        p = strtok((char*)receivedMessage, ",");		// p = UTC
        
        p = strtok(NULL, ",");			                // P = latitude
        strncpy(latitude, p, strlen(p));
        
        p = strtok(NULL, ",");			                // P = longitude
        strncpy(longitude, p, strlen(p));
	}
	else
	{
		return 0;
	}
    
    // Turn off GNSS
	n = sprintf((char*)&blinkMessage[0], "AT+QGPSEND\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Turn off GPS!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}
}

int sendSendSensorData()
{
	// Set Will to 0
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"will\",0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set Will to 0!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Set Timeout to 60
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"timeout\",0,60,3,0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set Timeout to 60!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Set Keep alive to 60
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCFG=\"keepalive\",0,60\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set keep alive to 60!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Open network
	//n = sprintf((char*)&blinkMessage[0], "at+qmtopen=0,\"ec2-54-226-136-203.compute-1.amazonaws.com\",1883\r\n");
	// 3.210.5.128
	n = sprintf((char*)&blinkMessage[0], "at+qmtopen=0,\"3.210.5.128\",1883\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);

	if (strstr((char*)receivedMessage, "+QMTOPEN: 0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Open Network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Connect network
	n = sprintf((char*)&blinkMessage[0], "AT+QMTCONN=0,\"clientExample\"\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QMTCONN: 0,0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Connect to a network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}


	// Publishing Sensor Data
	n = sprintf((char*)&blinkMessage[0], "AT+QMTPUB=0,0,0,0,\"william/topic1\"\r\n");
	__HAL_UART_CLEAR_IT(&hlpuart1, UART_CLEAR_NEF|UART_CLEAR_OREF);
	// HAL_UART_Receive_IT(&hlpuart1, receiveData, 500);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(500);

    // Upload Stored Sensor Data, Status, Battery Voltage
    n = sprintf((char*)&blinkMessage[0], "=>Uploading stored sensor data, status and battery voltage.\r\n");
    CDC_Transmit_FS(blinkMessage, n);

	uint8_t i = 0;
	n = sprintf((char*)&blinkMessage[0], "SP");

	
	n = sprintf((char*)&blinkMessage[0], "%s EP", blinkMessage);
    CDC_Transmit_FS(blinkMessage, n);
	end = 0;


	// n = sprintf((char*)&blinkMessage[0], "this is test");
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(500);
	uint8_t ctrl_z = 0x1A;
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, &ctrl_z, 1, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QMTPUB: 0,0,0") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "publish data to a server!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Disconnect network
	n = sprintf((char*)&blinkMessage[0], "AT+QMTDISC=0\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Disconnect from a network!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}
	return 1;
}

int receiveConfigurationData()
{
	// Set context id for HTTP connection
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPCFG=\"contextid\",1\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

 	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Set context id for HTTP connection!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Set context id failed!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}


	// Check QIACT for HTTP connection
	n = sprintf((char*)&blinkMessage[0], "AT+QIACT?\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "PDP Context is new!\r\n");
		CDC_Transmit_FS(blinkMessage, n);

		// Set up PDP context
		n = sprintf((char*)&blinkMessage[0], "AT+CGDCONT=1,\"IPV4V6\",\"\",\"0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0\",0,0,0,0\r\n");
		hlpuart1.RxState = HAL_UART_STATE_READY;
		HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
		HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
		HAL_Delay(1000);

		if (strstr((char*)receivedMessage, "OK") != NULL)
		{
			n = sprintf((char*)&blinkMessage[0], "Set PDP context successfully!\r\n");
			CDC_Transmit_FS(blinkMessage, n);
		}
		else
		{
			return 0;
		}
	}
	else if (strstr((char*)receivedMessage, "+QIACT: 1,1,1") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "PDP context is already set!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// Set HTTP url for HTTP connection
	//n = sprintf((char*)&blinkMessage[0], "AT+QHTTPURL=86,80\r\n");
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPURL=59,80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));
// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "CONNECT") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP connect!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	//n = sprintf((char*)&blinkMessage[0], "http://ec2-54-226-136-203.compute-1.amazonaws.com/api/v1/modem/123/mcu_config_download\r\n");
	n = sprintf((char*)&blinkMessage[0], "http://54.226.136.203/api/v1/modem/123/mcu_config_download/\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));
	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Connection to AWS is OK!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "Connection to AWS for config is not OK!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// HTTP Get
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPGET=80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));
	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QHTTPGET: 0,200") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP Get is working!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP Server is not available!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// HTTP Get
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPREAD=80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));
	// CDC_Transmit_FS(blinkMessage, receivedLen);

	if (strstr((char*)receivedMessage, "CONNECT") != NULL)
	{
		if (strstr((char*)receivedMessage, "SP") != NULL && strstr((char*)receivedMessage, "EP") != NULL)
			extractMCUConfigInfo();
	}
	else
	{
		return 0;
	}


	HAL_Delay(500);
	n = sprintf((char*)&blinkMessage[0], "\r\nPreparing for download config!\r\n");
	CDC_Transmit_FS(blinkMessage, n);

	// MCU update config
	// Set HTTP url for HTTP connection
	//n = sprintf((char*)&blinkMessage[0], "AT+QHTTPURL=73,80\r\n");
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPURL=45,80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);

	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));
	if (strstr((char*)receivedMessage, "CONNECT") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP connect!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	//n = sprintf((char*)&blinkMessage[0], "http://ec2-54-226-136-203.compute-1.amazonaws.com/api/v1/modem/123/update\r\n");
	n = sprintf((char*)&blinkMessage[0], "http://54.226.136.203/api/v1/modem/123/update\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(1000);
	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));

	if (strstr((char*)receivedMessage, "OK") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "Connection to AWS is OK!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}

	// HTTP Get
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPGET=80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);
	// CDC_Transmit_FS(receivedMessage, strlen((char *)receivedMessage));

	// CDC_Transmit_FS(blinkMessage, receivedLen);
	if (strstr((char*)receivedMessage, "+QHTTPGET: 0,200") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP Get is working!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP Server is not available!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
		return 0;
	}

	// HTTP Get
	n = sprintf((char*)&blinkMessage[0], "AT+QHTTPREAD=80\r\n");
	hlpuart1.RxState = HAL_UART_STATE_READY;
	HAL_UART_Receive_IT(&hlpuart1, receivedMessage, 200);
	HAL_UART_Transmit(&hlpuart1, blinkMessage, n, 1000);
	HAL_Delay(3000);

	// CDC_Transmit_FS(receivedMessage, strlen((char*)receivedMessage));

	if (strstr((char*)receivedMessage, "CONNECT") != NULL)
	{
		n = sprintf((char*)&blinkMessage[0], "HTTP READ is OK!\r\n");
		CDC_Transmit_FS(blinkMessage, n);
	}
	else
	{
		return 0;
	}


	return 1;
}
