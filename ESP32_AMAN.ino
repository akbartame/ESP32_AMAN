
/*
 * This Arduino Sketch controls a vibration detector and a smoke detector.
 * If vibration is detected, it alerts the user via serial print and web.
 * If smoke is detected, it activates an exhaust fan through a solid-state relay.
 * The exhaust fan can also be controlled via web, and the DC motor can only
 * be controlled via web. It also sets up a web server to monitor the status
 * and control the fan and motor. Additionally, it uses LEDs to indicate the
 * status: green for no danger, yellow for partial danger, and red for full danger.
 */


#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ElegantOTA.h>
#include <LittleFS.h>
#include <time.h>
#include <WiFiManager.h>
#include "declaration.h"

#include <FirebaseESP32.h>
// #include <FirebaseJson.h>
// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>
/* 2. If work with RTDB, define the RTDB URL and database secret */
#define DATABASE_URL "aman-8a19f-default-rtdb.asia-southeast1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app
#define DATABASE_SECRET "2v7CbNgfKGeZ3KDRwc0Bi46YYXzbtg6zBD8E5Df6"

FirebaseData fbdo;
FirebaseJson jsonData;  // Firebase JSON object to hold the data

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

// Create a web server object
AsyncWebServer server(80);
// Create a WiFiManager Object
WiFiManager wm;

// NTP Server details
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600; // Adjust for your timezone (e.g., GMT+7 = 7 * 3600)
const int daylightOffset_sec = 0; // Adjust for daylight saving if needed

#include <EEPROM.h>
#define EEPROM_SIZE 32

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  EEPROM.begin(EEPROM_SIZE);
  for (int i = 0; i < EEPROM_SIZE; i++) {
    if (EEPROM.read(i) >= 255) {
    EEPROM.write(i, 0);
    EEPROM.commit();
    }
  }
  // Inisialisasi pin input
  pinMode(vibrationSensorPin1, INPUT_PULLUP);
  pinMode(vibrationSensorPin2, INPUT_PULLUP);
  pinMode(vibrationSensorPin3, INPUT_PULLUP);
  pinMode(vibrationSensorPin4, INPUT_PULLUP);

  // Inisialisasi limit switch dengan pull-up internal
  pinMode(limitSwitch1, INPUT_PULLUP);
  pinMode(limitSwitch2, INPUT_PULLUP);

  // Inisialisasi LED, buzzer, dan relay sebagai output
  pinMode(ledHijau, OUTPUT);
  pinMode(ledKuning, OUTPUT);
  pinMode(ledMerah, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  pinMode(relayExhaust, OUTPUT);

  // Set ADC attenuation untuk pembacaan hingga 3.3V
  analogSetAttenuation(ADC_11db);
  
  // Web Setup
  wm.setConfigPortalTimeout(60);
  initWifiManager(); // GR LED turning on indicate wifi fail to connect, will restart in 3
  initLittleFS();
  webHandle();
  ElegantOTA.begin(&server);    // Start ElegantOTA
  Serial.println("CONNECTED.. YEY");

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  
  // RTC
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for NTP time...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Time synchronized!");
  
  // MQ2 PRE HEAT
  gasSensorPreheat(); // about 1 min the RY LED will turn on
  lastResetMonth = EEPROM.read(0);          // read from flash
  readUIntArrayFromEEPROM(1, vibrationCount, 4);

  //FIREBASE
  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);
  /* Assign the database URL and database secret(required) */
  config.database_url = DATABASE_URL;
  config.signer.tokens.legacy_token = DATABASE_SECRET;
  // config.signer.test_mode = true;

  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);

  /* Initialize the library with the Firebase authen and config */
  Firebase.begin(&config, &auth);
  Firebase.reconnectNetwork(true);

  // initial get state on firebase
  safeGetBool(fbdo, "/deviceId/control/switch_buzzer", prevBuzzerState);
  safeGetBool(fbdo, "/deviceId/control/switch_pintu", prevDoorState);
  safeGetBool(fbdo, "/deviceId/control/switch_exhaust", prevExhaustState);
}

void loop() {

  ElegantOTA.loop();
  readSensorAll();  autoGas();  autoVib();  timeoutManualOverride();

  //isi yang ada di function_AMAN.ino
  if (millis() - previousMillis >= 5000) {
    previousMillis = millis(); // Perbarui waktu terakhir
    writeSensorAll(); // Panggil fungsi yang diterima sebagai parameter
  }

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000))
  {
    sendDataPrevMillis = millis();
    sendSensorDataToFirebase();
    fetchControlCommandsFromFirebase();
  }

}

