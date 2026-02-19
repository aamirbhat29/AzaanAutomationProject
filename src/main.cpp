#include <Arduino.h>
#include "WiFiManager.h"
#include "AzaanTimes.h"
#include "TimeManager.h"
#include "LCDManager.h"
#include "DFPlayerManager.h"
#include "LocationManager.h"
#include "RCTManager.h"
#include "DHT11Sensor.h"
#include "LEDManager.h"

// Global Variables
unsigned long lastLCDUpdateTime = 0;
unsigned long lastAzaanCheckTime = 0;
String currentTime = "";

void setup()
{
  Serial.begin(115200);
  Serial.println("\n\n=== ESP32 Azaan System Starting ===");

  // Initialize the LCD
  initializeLCD();
  displayMessage("Welcome...");
  delay(1000);

  // Initialize RTC (may fail if not connected, that's okay)
  initializeRTC();

  // Initialize LED Manager
  beginLEDManager();

  // Attempt to connect to Wi-Fi
  displayMessage("WiFi...");
  setupWiFi();

  // Sync time with NTP
  displayMessage("Time Sync...");
  syncTimeWithMultipleServers();

  if (timeSynced)
  {
    displayMessage("Time Synced!");
    Serial.println("✓ Time synchronized successfully");
  }
  else
  {
    displayMessage("RTC Fallback");
    Serial.println("⚠ Using RTC time");
  }
  delay(1500);

  // Initialize DFPlayer
  displayMessage("Audio Init...");
  setupDFPlayer();
  delay(1000);

  // *** FETCH PRAYER TIMES ON BOOT ***
  displayMessage("Get Prayers...");
  Serial.println("\n=== Fetching Prayer Times on Boot ===");
  fetchPrayerTimes();
  delay(2000);

  // *** INITIALIZE PRAYER NAME/TIME FOR DISPLAY ***
  checkAndTriggerAzaan(); //
  Serial.println("Prayer name set for display");

  // Clear LCD for main display
  lcd.clear();
  Serial.println("=== Setup Complete ===");
  Serial.println("Entering main loop...\n");
}

void loop()
{
  // Wifi LED State control
  handleWifiLEDState();

  // Periodic time sync
  handleTimeSync();

  unsigned long currentMillis = millis();

  // Update LCD every second
  if (currentMillis - lastLCDUpdateTime >= 1000)
  {
    lastLCDUpdateTime = currentMillis;

    currentTime = getCurrentTime();
    Serial.print("[LOOP] Time: ");
    Serial.println(currentTime);

    displayTime(currentTime);
    displayDHT11Data();
  }

  // Check for Azaan every 10 seconds
  if (currentMillis - lastAzaanCheckTime >= 10000)
  {
    lastAzaanCheckTime = currentMillis;
    checkAndTriggerAzaan();
  }
}