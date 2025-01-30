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

void setup() {
  Serial.begin(115200);

  // Initialize the LCD
  initializeLCD();

  // Print a welcome message on the LCD
  displayMessage("Welcome...");
  //pinMode(14, OUTPUT);
  initializeRTC();

  beginLEDManager();  // Initialize LED pin
  setupWiFi();        // Attempt to connect to Wi-Fi

  // Sync time
  syncTimeWithMultipleServers();

  // Update LCD based on the time sync status
  if (timeSynced) {
    displayMessage("Time Synced!");
  } else {
    displayMessage("RTC Fallback");
  }
   delay(2000);  // Show the status message for 2 seconds

  // Start displaying time on the LCD
  lcd.clear();  // Clear the LCD screen before showing time
  // Display the current time from RTC (or NTP if synced)
 

//uncomment after adding battery to RTC module
  // Put ESP32 into deep sleep mode to save power
  // Serial.println("Going to deep sleep...");
  // esp_sleep_enable_timer_wakeup(DEEP_SLEEP_TIME);
  // esp_deep_sleep_start();

  // Initialize DFPlayer
  setupDFPlayer();

  // Sync time
  //syncTimeWithMultipleServers();

  // Fetch prayer times
  //fetchPrayerTimes();

  // Testing Azaan
  //playAzaanDemoTest();
}

void loop() {
 //Wifi LED State control
 handleWifiLEDState();

 // Time sync with multiple NTP servers
   handleTimeSync();
  unsigned long currentMillis = millis();

  // Update LCD every second
  if (currentMillis - lastLCDUpdateTime >= 1000) {
    lastLCDUpdateTime = currentMillis;
     currentTime = getCurrentTime();  // Update the current time
    displayTime(currentTime);
    //displayPT();
      // Read DHT11 data and display with scrolling
    displayDHT11Data();  // Display sensor data on the second row
    
  }

  // Check for Azaan every 10 seconds
  if (currentMillis - lastAzaanCheckTime >= 10000) {
    lastAzaanCheckTime = currentMillis;
    checkAndTriggerAzaan();
  }
}
