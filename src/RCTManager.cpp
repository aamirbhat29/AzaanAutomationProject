#include "RCTManager.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RTClib.h>

// RTC and NTP client initialization
RTC_DS3231 rtc;
WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 0, 60000);

// Track if RTC is actually available
bool rtcAvailable = false;

// Initialize the RTC
void initializeRTC()
{
  Serial.println("Checking for RTC module...");

  if (!rtc.begin())
  {
    Serial.println("⚠ RTC not detected - will rely on NTP time only");
    rtcAvailable = false;
    return;
  }

  rtcAvailable = true;
  Serial.println("✓ RTC module detected");

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, will sync with NTP when available");
  }
}

// Set RTC with compile-time if no sync is found
void setRTC()
{
  if (!rtcAvailable)
  {
    Serial.println("RTC not available, cannot set time");
    return;
  }

  rtc.adjust(DateTime(2025, 1, 1, 0, 0, 0));
  Serial.println("RTC time set to default.");
}

// Sync RTC time with NTP
void syncTimeWithNTP()
{
  if (!rtcAvailable)
  {
    // RTC not available, nothing to sync
    return;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      Serial.println("Syncing NTP time to RTC...");
      rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                          timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                          timeinfo.tm_sec));
      Serial.println("✓ RTC synchronized with NTP");
    }
    else
    {
      Serial.println("Failed to get NTP time for RTC sync");
    }
  }
}

// Get the current RTC time (with safety check)
DateTime getCurrentRTC()
{
  if (!rtcAvailable)
  {
    // Return a default/invalid time if RTC not available
    // The calling code should check timeSynced flag instead
    return DateTime(2000, 1, 1, 0, 0, 0);
  }

  return rtc.now();
}

// Initialize RTC and get the current time
DateTime initializeAndGetRTC()
{
  initializeRTC();

  if (!rtcAvailable)
  {
    Serial.println("RTC not available, returning default time");
    return DateTime(2000, 1, 1, 0, 0, 0);
  }

  return getCurrentRTC();
}

// Check if RTC is available
bool isRTCAvailable()
{
  return rtcAvailable;
}
