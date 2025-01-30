#include "TimeManager.h"
#include <TimeLib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino.h>
#include "RCTManager.h"

// Global variables
bool timeSynced = false;              // Global flag to track if time was successfully synchronized
unsigned long lastRetryTime = 0;      // Last retry timestamp
const unsigned long retryInterval = 600000; // 10 minutes in milliseconds
unsigned long lastSyncTime = 0;             // Timestamp of the last synchronization
const unsigned long syncInterval = 86400000; // 24 hours in milliseconds

const char* ntpServer = "pool.ntp.org"; 
const char* ntpServers[] = {"time.nist.gov", "pool.ntp.org", "time.google.com"};
const int numNTPServers = 3;
const long gmtOffset_sec = 19800;      // GMT+5:30 (India Standard Time)
const int daylightOffset_sec = 0;     // No daylight saving in India

// Sync time with multiple servers
void syncTimeWithMultipleServers() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected. Falling back to RTC time.");
        updateRTC();  // Fallback to RTC time if Wi-Fi is not connected
        return;
    }

    for (int i = 0; i < numNTPServers; i++) {
        Serial.print("Trying to sync time with NTP server: ");
        Serial.println(ntpServers[i]);

        configTime(gmtOffset_sec, daylightOffset_sec, ntpServers[i]);
        unsigned long syncStartTime = millis();
        while (millis() - syncStartTime < 5000) { // Try for 5 seconds
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                Serial.println("Time synchronized successfully!");
                Serial.printf("Time: %02d:%02d:%02d\n", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

                // Update the RTC with the synchronized time
                rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                                    timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));

                timeSynced = true;
                lastSyncTime = millis(); // Update the last sync timestamp
                return;  // Exit function on successful synchronization
            }
        }
    }

    // If all NTP attempts fail
    Serial.println("Unable to sync time after multiple attempts. Falling back to RTC time.");
    updateRTC();
}

// Update RTC fallback
void updateRTC() {
    // Fetch and display the current RTC time
    DateTime now = rtc.now();
    Serial.println("Using RTC time as fallback:");
    Serial.printf("Time: %02d:%02d:%02d\n", now.hour(), now.minute(), now.second());
}

// Get the current time (with RTC fallback if necessary)
String getCurrentTime() {
    if (!timeSynced) {
        // Get time from RTC if NTP sync fails or is unavailable
        DateTime now = getCurrentRTC();  // Assuming this function fetches the time from your RTC
        char rtcTimeStr[9];
        sprintf(rtcTimeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        return String(rtcTimeStr);
    }

    struct tm timeinfo;
    unsigned long startTime = millis();
    while (!getLocalTime(&timeinfo) && millis() - startTime < 5000) {
        delay(100);  // Retry every 100ms for up to 5 seconds
    }

    if (!getLocalTime(&timeinfo)) {
        // If getLocalTime fails, use RTC as fallback
        DateTime now = getCurrentRTC();  // Fetch time from RTC
        char rtcTimeStr[9];
        sprintf(rtcTimeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
        return String(rtcTimeStr);
    }

    // Format the time string (e.g., "05:30:00")
    char timeStr[9];  // Format: HH:MM:SS
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    return String(timeStr);
}

// Call this in loop() to handle periodic synchronization
void handleTimeSync() {
  // Check if it's time to sync with NTP servers
  if (millis() - lastSyncTime > syncInterval) {
    Serial.println("Attempting to sync time with NTP servers...");
    syncTimeWithMultipleServers();
    lastSyncTime = millis();  // Update the last sync time
  }
}
