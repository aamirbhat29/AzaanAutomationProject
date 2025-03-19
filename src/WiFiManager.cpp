#include "WiFiManager.h"
#include <WiFi.h>
#include "LEDManager.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Wi-Fi Credentials
const char* ssid = "Airtel_aami_8816";
const char* password = "air88659"; // Wrong password for testing

// const char* ssid = "ACTFIBERNET";
// const char* password = "act12345"; // Wrong password for testing

// Web Server on Port 80
AsyncWebServer server(80);

bool wifiConnecting = false;  // Wi-Fi connection attempt flag

void setupWiFi() {
  //Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.println("Connecting to Wi-Fi...");

  unsigned long startAttemptTime = millis();
  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED && retryCount < 3) {
    //Serial.println("Blinking LED to indicate Wi-Fi connection attempt...");
    blinkLED(wifiPin, 500);  // Blink LED during connection attempts

    if (millis() - startAttemptTime >= 30000) {  // 30 seconds per attempt
      Serial.println("Retrying Wi-Fi connection...");
      WiFi.begin(ssid, password);  // Reattempt connection
      startAttemptTime = millis();
      retryCount++;
    }
    delay(100);  // Small delay for blinking
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected!");
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    setWiFiStatus(true);  // Solid LED on success

    // Start the Web Server
    setupWebServer();
  } else {
    Serial.println("Failed to connect to Wi-Fi after retries.");
    setWiFiStatus(false);  // LED off or blinking
  }
}

void setupWebServer() {
  Serial.println("Setting up web server...");

  // Handle Volume Control
  server.on("/volume", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("level")) {
      String volumeLevel = request->getParam("level")->value();
      Serial.println("Volume Set To: " + volumeLevel);
      request->send(200, "text/plain", "Volume set to " + volumeLevel);
    } else {
      request->send(400, "text/plain", "Missing 'level' parameter");
    }
  });

  // Handle Status Request
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    StaticJsonDocument<200> doc;
    doc["battery"] = 85;  // Example battery level
    doc["temperature"] = 25.3; // Example temperature

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });

  server.begin();
  Serial.println("Web server started.");
}
