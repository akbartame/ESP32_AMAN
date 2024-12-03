/*place this on void loop()
if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();
    sendSensorDataToFirebase();
    fetchControlCommandsFromFirebase();
    count++;
  }
*/

void sendSensorDataToFirebase() {
  struct tm timeinfo;
  char buffer[64];
  if (getLocalTime(&timeinfo)) strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  // Create and clear the JSON object for storing sensor data
  jsonData.clear();

  // Add the sensor data to the JSON object
  jsonData.set("/gas_value", gasValue);
  jsonData.set("/vib_value/S1", sensorValue1);
  jsonData.set("/vib_value/S2", sensorValue2);
  jsonData.set("/vib_value/S3", sensorValue3);
  jsonData.set("/vib_value/S4", sensorValue4);
  jsonData.set("/vib_total/S1", vibrationCount[0]);
  jsonData.set("/vib_total/S2", vibrationCount[1]);
  jsonData.set("/vib_total/S3", vibrationCount[2]);
  jsonData.set("/vib_total/S4", vibrationCount[3]);
  jsonData.set("/status", kondisi);
  jsonData.set("/vib_total/timestamp", buffer);
  
  // Sending the entire JSON object to Firebase in one operation
  Serial.printf("Sending sensor data to Firebase... %s\n", 
                 Firebase.updateNode(fbdo, "/deviceId/monitor", jsonData) ? "Success" : fbdo.errorReason().c_str());
}

bool safeGetBool(FirebaseData &fbdo, const char *path, bool &outputVar) {
  if (Firebase.get(fbdo, path)) {
    // Check if the data is a boolean
    if (fbdo.dataType() == "boolean") {
      outputVar = fbdo.boolData();
      return true;
    } else if (fbdo.dataType() == "int") {
      // Convert integer (non-zero -> true, zero -> false)
      outputVar = fbdo.intData() != 0;
      return true;
    } else if (fbdo.dataType() == "string") {
      // Interpret string ("true" or "1" -> true)
      String strValue = fbdo.stringData();
      if (strValue.equalsIgnoreCase("true") || strValue == "1") {
        outputVar = true;
      } else {
        outputVar = false;
      }
      return true;
    } else {
      // Log unsupported data type
      Serial.printf("Unsupported data type at %s: %s\n", path, fbdo.dataType().c_str());
    }
  } else {
    // Log error
    Serial.printf("Error reading %s: %s\n", path, fbdo.errorReason().c_str());
  }
  return false; // Indicate failure
}

void fetchControlCommandsFromFirebase() {
  bool buzzerState, doorState, doorControl, exhaustState;

  // Fetch buzzer control
  if (safeGetBool(fbdo, "/deviceId/control/switch_buzzer", buzzerState)) {
      if (buzzerState != prevBuzzerState) { // Check for change
      Serial.printf("Buzzer control changed to: %s\n", buzzerState ? "ON" : "OFF");
      manualControl(buzzer, buzzerState);
      prevBuzzerState = buzzerState; // Update previous state
    }
    Serial.println("Feeding BUZZER data... OK");
  }
  else {Serial.printf("Feeding BUZZER data from Firebase failed : %s\n", fbdo.errorReason().c_str());}

  // Fetch door control
  if (safeGetBool(fbdo, "/deviceId/control/switch_pintu", doorState)) {
    if (doorState != prevDoorState) { // Check for change
      Serial.printf("Door control changed to: %s\n", doorState ? "OPEN" : "CLOSED");
      lastManualControlTime = millis(); // Update timer with the current time
      manualDoorControl = true;
      if (doorState) {manualDoorState = true;}
      else if (!doorState) {manualDoorState = false;}
      Serial.printf("Web : ===Door Controlled %s===\n",doorState ? "ON" : "OFF");
      prevDoorState = doorState; // Update previous state
    }
    Serial.println("Feeding DOOR data... OK");
  }
  else {Serial.printf("Feeding DOOR data from Firebase failed : %s\n", fbdo.errorReason().c_str());}

  if (safeGetBool(fbdo, "/deviceId/control/switch_kontrolPintu", doorControl)) {
    if (doorControl) { 
      Serial.printf("Door manual changed to: %s\n", doorControl ? "YES" : "NO");
      manualDoorControl = true;
      dontAutoDoor = true;
      Serial.printf("Web : ===Door FULL Manually Controlled ON===\n");
    }
    else {
      dontAutoDoor = false;
      Serial.printf("Web : ===Door FULL Manually Controlled OFF===\n");
    }
    Serial.println("Feeding DOOR data... OK");
  }
  else {Serial.printf("Feeding DOOR data from Firebase failed : %s\n", fbdo.errorReason().c_str());}

  // Fetch exhaust control
  if (safeGetBool(fbdo, "/deviceId/control/switch_exhaust", exhaustState)) {
    if (exhaustState != prevExhaustState) { // Check for change
      Serial.printf("Exhaust control changed to: %s\n", exhaustState ? "ON" : "OFF");
      manualControl(exhaust, exhaustState);
      prevExhaustState = exhaustState; // Update previous state
    }
    Serial.println("Feeding EXHAUST data... OK");
  }
  else {Serial.printf("Feeding EXHAUST data from Firebase failed : %s\n", fbdo.errorReason().c_str());}
}
