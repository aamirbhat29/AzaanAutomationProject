#ifndef RCT_MANAGER_H
#define RCT_MANAGER_H

#include <Wire.h>
#include <RTClib.h>

// Initialize RTC object (e.g., DS3231)
extern RTC_DS3231 rtc;

void initializeRTC();          // Function to initialize RTC
void setRTC();                 // Function to set RTC time
void updateRTC();              // Function to update RTC time
DateTime getCurrentRTC();      // Function to get the current time from RTC
void syncTimeWithNTP();        // Function to sync time with NTP if Wi-Fi is available
DateTime initializeAndGetRTC();

#endif
