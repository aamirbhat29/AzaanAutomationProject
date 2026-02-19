#include "WiFiManager.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include "LEDManager.h"

// Create WiFiMulti instance
WiFiMulti wifiMulti;

// Add your WiFi networks here - it will auto-connect to the strongest one
// Format: wifiMulti.addAP("SSID", "PASSWORD");
void setupWiFiNetworks()
{
  // Home Network 1
  wifiMulti.addAP("Airtel_aami_8816", "air66539");

  // Home Network 2 (add your second WiFi here)
  wifiMulti.addAP("JIO-Aamir", "jio@2912");

  // Office/Mosque WiFi (optional)
  // wifiMulti.addAP("MosqueWiFi", "password3");

  // Friend's house (optional)
  // wifiMulti.addAP("FriendWiFi", "password4");

  // Mobile hotspot backup (optional)
  // wifiMulti.addAP("YourPhone", "hotspotpass");
}

bool wifiConnecting = false;

void setupWiFi()
{
  Serial.println("=== WiFi Multi-Network Setup ===");

  // Add all your networks
  setupWiFiNetworks();

  Serial.println("Scanning for available networks...");
  Serial.println("Will connect to strongest signal...");

  unsigned long startAttemptTime = millis();
  int attempts = 0;
  const int maxAttempts = 3;

  while (attempts < maxAttempts)
  {
    Serial.print("Connection attempt ");
    Serial.print(attempts + 1);
    Serial.print("/");
    Serial.println(maxAttempts);

    // Try to connect (timeout 10 seconds)
    if (wifiMulti.run(10000) == WL_CONNECTED)
    {
      Serial.println("\n✓ WiFi Connected!");
      Serial.print("Connected to: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      Serial.print("Signal Strength: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");

      setWiFiStatus(true);
      return;
    }

    Serial.println("Failed, retrying...");
    blinkLED(wifiPin, 500);
    attempts++;
    delay(2000);
  }

  Serial.println("\n✗ Failed to connect to any WiFi network");
  Serial.println("Continuing in offline mode...");
  setWiFiStatus(false);
}

// Call this in loop to maintain connection
void maintainWiFiConnection()
{
  // Check connection every loop
  if (wifiMulti.run() != WL_CONNECTED)
  {
    Serial.println("WiFi disconnected, reconnecting...");
    blinkLED(wifiPin, 500);
  }
  else
  {
    setWiFiStatus(true);
  }
}