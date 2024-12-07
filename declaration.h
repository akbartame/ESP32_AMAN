unsigned int lastResetMonth = 0;          // Stores the month of the last reset
unsigned long lastManualControlTime = 0;  // Tracks last manual control time
unsigned long previousMillis = 0; // Waktu terakhir fungsi dipanggil
unsigned long previousMillisWiFi = 0;
const int maxReconnectAttempts = 5; // Maximum reconnection attempts
int reconnectAttempts = 0;          // Counter for reconnect attempts

#define AO_PIN 34  // ESP32's pin GPIO34 connected to AO pin of the MQ2 sensor
int gasValue = 0;
int batasBawah = 700;
int batasAtas = 1500;
int kondisi = 0;
bool LS1nyala = 0;
bool LS2nyala = 0;
bool isWaiting = false;

unsigned int vibrationCount[4] = {0, 0, 0, 0};
bool sensorValue1 = 0;
bool sensorValue2 = 0;
bool sensorValue3 = 0;
bool sensorValue4 = 0;

// Pin untuk sensor getaran
const int vibrationSensorPin1 = 12;
const int vibrationSensorPin2 = 13;
const int vibrationSensorPin3 = 23;
const int vibrationSensorPin4 = 15;

// Pin untuk LED dan Buzzer
const int ledHijau = 18;
const int ledKuning = 25;
const int ledMerah = 21;
const int buzzerPin = 22;

// Pin untuk Relay SSR
const int relayPin1 = 26;  // Relay SSR 1 untuk Forward (Buka jendela)
const int relayPin2 = 27;  // Relay SSR 2 untuk Reverse (Tutup jendela)
const int relayExhaust = 14; // Relay untuk exhaust
// Variables to store the state of the relays
bool relay1State = false;
bool relay2State = false;

// Pin untuk Limit Switch
const int limitSwitch1 = 35; // Limit switch untuk forward (Buka jendela)
const int limitSwitch2 = 33; // Limit switch untuk reverse (Tutup jendela)
// const int limitSwitch2 = 32;
// const int ledKuning = 19;

// Manual control flags
bool manualBuzzerControl = false;

bool manualDoorControl = false;
bool manualDoorState = false;
bool dontAuto = false;
bool doorIsOpen = false;
unsigned long lastOpen;

bool manualExhaustControl = false;

// Previous states to track changes
bool prevBuzzerState = false;
bool prevDoorState = false;
bool prevExhaustState = false;

//mode 
#define exhaust 0
#define buzzer 1
