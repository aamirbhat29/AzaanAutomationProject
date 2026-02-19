#include "WiFiManager.h"
#include <WiFi.h>
#include "LEDManager.h"

const char *ssid = "JIO-Aamir";
const char *password = "jio@2912"; // VERIFY THIS PASSWORD IS CORRECT!

bool wifiConnecting = false; // Flag to indicate Wi-Fi connection attempt

void setupWiFi()
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  int retryCount = 0;

  // REDUCED: 2 retries × 10 seconds = 20 seconds max (was 90 seconds!)
  while (WiFi.status() != WL_CONNECTED && retryCount < 2)
  {
    blinkLED(wifiPin, 500); // Blink LED during connection attempts

    if (millis() - startAttemptTime >= 10000)
    { // CHANGED: 10 seconds per attempt (was 30)
      Serial.println("Retrying Wi-Fi connection...");
      WiFi.begin(ssid, password); // Reattempt connection
      startAttemptTime = millis();
      retryCount++;
    }
    delay(100); // Small delay for blinking
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Wi-Fi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    setWiFiStatus(true); // Solid light on success
  }
  else
  {
    Serial.println("Failed to connect to Wi-Fi after retries. Continuing with RTC/offline mode.");
    setWiFiStatus(false); // LED turns off or stays blinking
  }
}
