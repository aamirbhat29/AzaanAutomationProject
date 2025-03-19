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
#include "DFRobotDFPlayerMini.h"
#include "watchDog.h"
#include <EEPROM.h>
#include <esp_sleep.h>

//#include <SoftwareSerial.h>

// Global Variables
//#define EEPROM_SIZE 1
//int dfplayerResetFlag = 0;  // Global variable for reset tracking
//RTC_DATA_ATTR static uint8_t boot_count = 0; // Stored in RTC memory
#define RESET_FLAG_ADDR 0  // EEPROM address for reset flag
RTC_DATA_ATTR int powerCycleFlag = 0;  // Resets on power-off, not on soft reset
unsigned long lastLCDUpdateTime = 0;
unsigned long lastAzaanCheckTime = 0;
String currentTime = "";
int trx = 0;
bool showingLocation = false;
unsigned long locationStartTime = 0;


void setup() {

  Serial.begin(115200);
  // EEPROM.begin(EEPROM_SIZE);

  //   if (EEPROM.read(0) == 255) {  // Uninitialized EEPROM check
  //       Serial.println("🔧 EEPROM is uninitialized! Setting default values.");
  //       dfplayerResetFlag = 0;
  //       EEPROM.write(0, dfplayerResetFlag);
  //       EEPROM.commit();
  //   }

  //   dfplayerResetFlag = EEPROM.read(0);
  //   Serial.print("🚀 Booting up! dfplayerResetFlag = ");
  //   Serial.println(dfplayerResetFlag);

  //   if (dfplayerResetFlag == 1) {
  //       Serial.println("✅ System restarted after DFPlayer failure. Resetting flag.");
  //       dfplayerResetFlag = 0;
  //       EEPROM.write(0, dfplayerResetFlag);
  //       EEPROM.commit();
  //       delay(500);
  //   }
  //startWatchdog();  // Start watchdog when ESP32 boots

  // myDFPlayer.volume(10);  //Set volume value. From 0 to 30
  setupDFPlayer();  //Play the first mp3

  // Initialize the LCD
  initializeLCD();

// // Initialize the IR remote
   initializeDHT(); 

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
  Serial.println("RESET_FLAG_ADDR - triggering auto-reset");
    Serial.println(RESET_FLAG_ADDR);
  EEPROM.begin(4);  // Initialize EEPROM

    int resetFlag = EEPROM.read(RESET_FLAG_ADDR);

    Serial.println("Stored Reset Flag: " + String(resetFlag));

    if (powerCycleFlag == 0 && resetFlag == 0) {  // This means the ESP has just been powered ON
        Serial.println("🔄 First power-on detected! Restarting ESP...");
        powerCycleFlag = 1;  // This will persist across soft resets but not power-off
        EEPROM.write(RESET_FLAG_ADDR, 1);
        EEPROM.commit();
        delay(1000);
        ESP.restart();  // Automatically restart ESP
    } else {
        Serial.println("✅ ESP Restarted. Continuing normal operation...");
        EEPROM.write(RESET_FLAG_ADDR, 0);  // Reset flag so it works after power cycle
        EEPROM.commit();
    }

  // Normal setup code (runs only after auto-reset)
  Serial.println("Post-reset initialization");
  // Sync time
  //syncTimeWithMultipleServers();

  // Fetch prayer times
  fetchPrayerTimes();

  // Testing Azaan
 // playAzaanDemoTest();
}


void loop() {
  

 //Wifi LED State control
 handleWifiLEDState();

 // Time sync with multiple NTP servers
   handleTimeSync();
  unsigned long currentMillis = millis();
  if (showingLocation) {
        // If 5 seconds have passed, return to original display
        if (currentMillis - locationStartTime >= 5000) {
            showingLocation = false;  // Reset flag
            lcd.clear();              // Clear display
        } else {
            return;  // Skip normal updates while showing location
        }
    }

  //Check for Azaan every 10 seconds
  if (currentMillis - lastAzaanCheckTime >= 10000) {
    lastAzaanCheckTime = currentMillis;
    checkAndTriggerAzaan();
  }
  
  // Update LCD every second
  if (currentMillis - lastLCDUpdateTime >= 1000) {
    lastLCDUpdateTime = currentMillis;
     currentTime = getCurrentTime();  // Update the current time
    displayTime(currentTime);
      // Read DHT11 data and display with scrolling
    displayDHT11Data();  // Display sensor data on the second row
    //feedWatchdog();  // Reset watchdog to prevent unnecessary restarts
  }
}