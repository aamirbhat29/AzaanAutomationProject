#include "WiFiManager.h"
#include <WiFi.h>
#include "LEDManager.h"

const char* ssid = "Airtel_aami_8816";
const char* password = "air88659"; // Wrong password for testing

bool wifiConnecting = false; // Flag to indicate Wi-Fi connection attempt

void setupWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  int retryCount = 0;

  while (WiFi.status() != WL_CONNECTED && retryCount < 3) {
    Serial.println("Blinking LED to indicate Wi-Fi connection attempt...");
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
    setWiFiStatus(true);  // Solid light on success
  } else {
    Serial.println("Failed to connect to Wi-Fi after retries.");
    setWiFiStatus(false);  // LED turns off or stays blinking
  }
}

