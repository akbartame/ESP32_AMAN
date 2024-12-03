void readSensorAll() {
  if ((WiFi.status() != WL_CONNECTED) && (millis() - previousMillisWiFi >= 3000)) {
  Serial.print(millis());
  Serial.println("Connection Lost: Reconnecting to WiFi...");
  WiFi.disconnect();
  WiFi.reconnect();
  previousMillisWiFi = millis();
  }
  gasValue = analogRead(AO_PIN);
  LS1nyala = !digitalRead(limitSwitch1);
  LS2nyala = !digitalRead(limitSwitch2);
  sensorValue1 = digitalRead(vibrationSensorPin1);
  sensorValue2 = digitalRead(vibrationSensorPin2);
  sensorValue3 = digitalRead(vibrationSensorPin3);
  sensorValue4 = digitalRead(vibrationSensorPin4);
}

void writeSensorAll() {
  // Display readings on Serial Monitor
  Serial.println("=== Real Time Clock ===");
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[64];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    Serial.printf("Current Time: %s\n", buffer);
  } else {
    Serial.println("Failed to obtain time");
  }
  
  Serial.println("=== Sensor Readings ===");
  Serial.print("Gas Value: ");
  Serial.println(gasValue);

  Serial.print("Limit Switch 1 (LS1): ");
  Serial.println(LS1nyala ? "ON" : "OFF");

  Serial.print("Limit Switch 2 (LS2): ");
  Serial.println(LS2nyala ? "ON" : "OFF");

  Serial.print("Vibration Sensor 1 (in 30 Days): ");
  Serial.println(vibrationCount[0]);

  Serial.print("Vibration Sensor 2 (in 30 Days): ");
  Serial.println(vibrationCount[1]);

  Serial.print("Vibration Sensor 3 (in 30 Days): ");
  Serial.println(vibrationCount[2]);

  Serial.print("Vibration Sensor 4 (in 30 Days): ");
  Serial.println(vibrationCount[3]);

  Serial.println("=======================");

  Serial.printf("Mode Buzzer: %s \n", manualBuzzerControl ? (prevBuzzerState ? "Manual (ON)" : "Manual (OFF)") : "Auto");
  Serial.printf("Mode Exhaust: %s \n", manualExhaustControl ?  (prevExhaustState ? "Manual (ON)" : "Manual (OFF)") : "Auto");
  Serial.printf("Mode Pintu: %s \n", manualDoorControl ?  (prevDoorState ? "Manual (ON)" : "Manual (OFF)") : "Auto");
  
  Serial.println("=======================");
}

void Door(bool mode) {
  digitalWrite(relayPin1, mode);
  digitalWrite(relayPin2, !mode);
}

void stopDoor() {
  digitalWrite(relayPin1, LOW);
  digitalWrite(relayPin2, LOW);
}

void exhaustOn(bool mode) { digitalWrite(relayExhaust, mode); }
void buzzerOn(bool mode) { digitalWrite(buzzerPin, mode); }

int gasCondition() {
  if (gasValue < 1500) { return 0; }
  else if (gasValue >= 1500 && gasValue < 2000) { return 1; }
  else { return 2; }
}


// Functions to open/close the door with manual override check
void OPENdoor() {
  if (!LS1nyala) { Door(true); } 
  else {
    stopDoor();
    // Serial.println("X Pintu berhenti membuka X");
  }
}

void CLOSEdoor() {
  if (!LS2nyala) { Door(false); }
  else {
    stopDoor();
    // Serial.println("X Pintu berhenti menutup X");
  }
}

void autoGas() {
  kondisi = gasCondition();

  // Update LEDs regardless of manual overrides
  digitalWrite(ledHijau, kondisi == 0);
  digitalWrite(ledKuning, kondisi == 1);
  digitalWrite(ledMerah, kondisi == 2);

  // Only update the buzzer and exhaust if not manually controlled
  if (!manualBuzzerControl) { buzzerOn(kondisi == 2); }
  if (!manualExhaustControl) { digitalWrite(relayExhaust, kondisi == 1); }

  // Door control based on gas condition, only if not manually controlled
  if (manualDoorControl) {
    // Manual control takes priority
    if (manualDoorState) OPENdoor();
    else CLOSEdoor();
  } else { 
    // Automatic control based on gas condition
    if (kondisi == 2) OPENdoor();
    else CLOSEdoor();
  }
}

void writeUIntArrayIntoEEPROM(int address, unsigned int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    EEPROM.write(addressIndex, (numbers[i] >> 8) & 0xFF); // High byte
    EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);    // Low byte
    addressIndex += 2;
  }
}

void readUIntArrayFromEEPROM(int address, unsigned int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = ((unsigned int)EEPROM.read(addressIndex) << 8) | EEPROM.read(addressIndex + 1);
    addressIndex += 2;
  }
}

// Monitor vibration sensors
bool sensorState[4] = {false, false, false, false};
const int debounceDelay = 50; // Waktu debounce dalam milidetik
unsigned long lastDebounceTime[4] = {0, 0, 0, 0};

void autoVib() {
  checkResetTime();
  bool eepromDirty;
  bool sensorValues[4] = {sensorValue1, sensorValue2, sensorValue3, sensorValue4};

  for (int i = 0; i < 4; i++) {
    if (sensorValues[i]) {
      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
        if (!sensorState[i]) {
          Serial.printf("Getaran terdeteksi oleh sensor %d\n", i + 1);
          sensorState[i] = true;
          if (vibrationCount[i] >= 0 && vibrationCount[i] < 65535) {vibrationCount[i]++;}
          else {vibrationCount[i] = 0;}
          eepromDirty = true;
          lastDebounceTime[i] = millis();
        }
      }
    } else {
      sensorState[i] = false;
      lastDebounceTime[i] = millis();
    }
  }
  if (eepromDirty) {
    writeUIntArrayIntoEEPROM(1, vibrationCount, 4);
    EEPROM.commit();
    eepromDirty = false;
  }
}


void checkResetTime() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  if (timeinfo.tm_mday == 1 && timeinfo.tm_mon != lastResetMonth) {
    // Reset the count on the 1st of a new month
    vibrationCount[0] = 0;
    vibrationCount[1] = 0;
    vibrationCount[2] = 0;
    vibrationCount[3] = 0;
    lastResetMonth = timeinfo.tm_mon; // Store the current month
    EEPROM.write(0, lastResetMonth);
    writeUIntArrayIntoEEPROM(1, vibrationCount, 4);
    EEPROM.commit();
    Serial.println("Vibration count reset for the new month!");
  }

}


// Manual control function with override activation
void manualControl(int thing, bool mode) {
  lastManualControlTime = millis(); // Update timer with the current time
  
  switch (thing) {
    case exhaust:
      manualExhaustControl = true;
      exhaustOn(mode);
      Serial.printf("Web : ===Exhaust Controlled %s===\n",mode ? "ON" : "OFF");
      break;

    case buzzer:
      manualBuzzerControl = true;
      buzzerOn(mode);
      Serial.printf("Web : ===Buzzer Controlled %s===\n",mode ? "ON" : "OFF");
      break;
  }
}

void timeoutManualOverride() {
  // Check if kondisi is 0 (safe state) and if 5 minutes have passed with no manual control
  if (millis() - lastManualControlTime >= 300000) {
    disableManualOverride();
    lastManualControlTime = millis();
  }
}

// Function to disable manual control overrides, restoring automatic behavior
void disableManualOverride() {
  manualExhaustControl = false;
  manualBuzzerControl = false;
  if (!dontAutoDoor) manualDoorControl = false;
  Serial.println("Web : ===Mode Auto ON===");
}

void gasSensorPreheat() {
  digitalWrite(ledHijau, LOW);
  digitalWrite(ledKuning, HIGH);
  digitalWrite(ledMerah, HIGH);
  Serial.println("Pre-heating MQ 2 Sensor wait about 1 min");
  delay(60000);
  digitalWrite(ledHijau, LOW);
  digitalWrite(ledKuning, LOW);
  digitalWrite(ledMerah, LOW);
  Serial.println("MQ 2 Sensor Ready");
}
