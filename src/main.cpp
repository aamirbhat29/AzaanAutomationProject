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
#include <WiFi.h>

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

  // Initialize RTC
  initializeRTC();

  // Initialize LED Manager
  beginLEDManager();

  // Initialize DHT11 sensor
  initializeDHT();

  // Connect to Wi-Fi
  displayMessage("WiFi...");
  setupWiFi();

  // Display connected WiFi SSID
  if (WiFi.status() == WL_CONNECTED)
  {
    String ssid = WiFi.SSID();
    displayMessage("WiFi: " + ssid);
    Serial.print("Connected to: ");
    Serial.println(ssid);
    delay(3000); // Show WiFi name for 3 seconds
  }
  else
  {
    displayMessage("WiFi Failed");
    delay(2000);
  }

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

  // Fetch prayer times on boot
  displayMessage("Get Prayers...");
  Serial.println("\n=== Fetching Prayer Times on Boot ===");
  fetchPrayerTimes();
  delay(2000);

  // Initialize prayer name/time for display
  checkAndTriggerAzaan();
  Serial.print("Next prayer: ");
  Serial.print(prayerName);
  Serial.print(" at ");
  Serial.println(currentPrayerTimeToDisplay);

  // Clear LCD for main display
  lcd.clear();
  Serial.println("=== Setup Complete ===");
  Serial.println("Entering main loop...\n");
}

void loop()
{
  // WiFi LED state control
  handleWifiLEDState();

  // Maintain WiFi connection
  maintainWiFiConnection();

  // Periodic time sync
  handleTimeSync();

  unsigned long currentMillis = millis();

  // Update LCD every second
  if (currentMillis - lastLCDUpdateTime >= 1000)
  {
    lastLCDUpdateTime = currentMillis;

    currentTime = getCurrentTime();
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