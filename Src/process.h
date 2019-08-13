
#define X_THRESHOLD		200
#define Y_THRESHOLD		200
#define Z_THRESHOLD		200

typedef enum adc_state
{
    BATTERY_STATE = 0,
    ASENSE_ADC1,
    ASENSE_ADC2,
    ASENSE_ADC3
} ADC_STATE;

typedef enum device_state
{
	POWER_CONNECT,
	SLEEP,
	WAKEUP,
	IGNITION_ON,
	IGNITION_OFF,
	MOTION,
	POWER_DISCONNECT,
	BATTERY_MODE,
	RELAY1_ON_OFF,
	RELAY2_ON_OFF,
	LIGHT_DETECTION
} DEVICE_STATE;

// Process when booting
// void RebootProcess();

// Process when measuring sensors
void SensorMeasureProcess();

// Process of communication with server
void IoTCommunicationProcess();

void BootBG96();
void ShutdownBG96();

void ReceiveMessage();

void extractMCUConfigInfo();

int connectNetwork();
int sendSendSensorData();
int getGPSData();
int receiveConfigurationData();

