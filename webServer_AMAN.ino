// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

void initWifiManager(){
  // Load values saved in LittleFS
  wm.setClass("invert");
  bool res;

  // wm.setSTAStaticIPConfig(IPAddress(192,168,1,99), IPAddress(192,168,1,1), IPAddress(255,255,255,0), IPAddress(8,8,8,8)); // optional DNS 4th argument
  res = wm.autoConnect("AMAN_AP","amanajadulu"); // password protected ap

  if(!res) {
    Serial.println("Failed to connect");
    digitalWrite(ledHijau, HIGH);
    digitalWrite(ledKuning, LOW);
    digitalWrite(ledMerah, HIGH);
    delay(3000);
    ESP.restart();
  }
}


void webHandle(){
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/index.html", "text/html", false);
    });
    server.serveStatic("/", LittleFS, "/");

    // Route for Monitoring page
    server.on("/monitoring.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(LittleFS, "/monitoring.html", "text/html", false);
    });

    // Route to reset WiFi (uses POST for security)
    server.on("/wifi-reset", HTTP_POST, [](AsyncWebServerRequest *request) {
        wm.resetSettings(); // Reset WiFi Manager settings
        ESP.restart();      // Restart ESP32
        request->send(200, "text/plain", "WiFi telah direset");
    });

    // Route to get sensor data (for Monitoring page)
    server.on("/sensor-data", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"gas\": " + String(gasValue) + 
                  ", \"vibration1\": " + String(sensorValue1) + 
                  ", \"vibration2\": " + String(sensorValue2) + 
                  ", \"vibration3\": " + String(sensorValue3) + 
                  ", \"vibration4\": " + String(sensorValue4) + 
                  ", \"vibration1Total\": " + String(vibrationCount[0]) +
                  ", \"vibration2Total\": " + String(vibrationCount[1]) +
                  ", \"vibration3Total\": " + String(vibrationCount[2]) +
                  ", \"vibration4Total\": " + String(vibrationCount[3]) +
                  ", \"status\": " + String(kondisi) + "\}"; // 0, 1, atau 2
    request->send(200, "application/json", json);
    });
    server.begin();
}


