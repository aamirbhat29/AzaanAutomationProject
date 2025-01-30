#include "RCTManager.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RTClib.h>

// RTC and NTP client initialization
RTC_DS3231 rtc;
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 0, 60000);  // NTP client setup, GMT offset 0 (for UTC)

// Initialize the RTC
void initializeRTC() {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC. Please check the connections.");
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting time.");
    setRTC();  // Set RTC to compile-time if power is lost
  }
}

// Set RTC with compile-time if no sync is found
void setRTC() {
  rtc.adjust(DateTime(2025, 1, 1, 0, 0, 0));  // Set RTC to compile time
  Serial.println("RTC time set to compile-time.");
}


// Sync RTC time with NTP
void syncTimeWithNTP() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // Set NTP servers
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      Serial.println("Time synchronized with NTP");
      rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                          timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                          timeinfo.tm_sec));
    } else {
      Serial.println("Failed to synchronize with NTP");
    }
  }
}

// Get the current RTC time
DateTime getCurrentRTC() {
  return rtc.now();  // Return current time from RTC
}

// New method to initialize RTC and get the current time
DateTime initializeAndGetRTC() {
  initializeRTC();  // Initialize RTC and set time if needed
  return getCurrentRTC();  // Return the current time from RTC after initialization
}
