#include "RCTManager.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <RTClib.h>

// RTC uses a SECOND I2C bus with swapped pins (22=SDA, 21=SCL)
TwoWire I2C_RTC = TwoWire(1); // Use I2C bus 1 for RTC
RTC_DS3231 rtc;

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 0, 60000);

bool rtcAvailable = false;

// Initialize the RTC on second I2C bus with swapped pins
void initializeRTC()
{
  Serial.println("Initializing RTC on I2C bus with SDA=22, SCL=21...");

  // Initialize second I2C bus with SWAPPED pins for RTC
  I2C_RTC.begin(22, 21); // SDA=22, SCL=21 (swapped from LCD's 21/22)
  delay(100);

  if (!rtc.begin(&I2C_RTC))
  {
    Serial.println("⚠ RTC not detected");
    rtcAvailable = false;
    return;
  }

  rtcAvailable = true;
  Serial.println("✓ RTC module detected and working!");

  // Print current RTC time
  DateTime now = rtc.now();
  Serial.print("RTC Time: ");
  Serial.print(now.year());
  Serial.print("-");
  Serial.print(now.month());
  Serial.print("-");
  Serial.print(now.day());
  Serial.print(" ");
  Serial.print(now.hour());
  Serial.print(":");
  Serial.print(now.minute());
  Serial.print(":");
  Serial.println(now.second());

  if (rtc.lostPower())
  {
    Serial.println("⚠ RTC lost power - time may be incorrect");
    Serial.println("RTC will be synced with NTP when WiFi connects");
  }
}

// Set RTC to a default time
void setRTC()
{
  if (!rtcAvailable)
  {
    Serial.println("RTC not available, cannot set time");
    return;
  }

  rtc.adjust(DateTime(2026, 2, 19, 12, 0, 0));
  Serial.println("RTC time set to default.");
}

// Sync RTC time with NTP
void syncTimeWithNTP()
{
  if (!rtcAvailable)
  {
    Serial.println("RTC not available, skipping NTP sync to RTC");
    return;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Syncing NTP time to RTC...");
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");
    delay(500);

    struct tm timeinfo;
    if (getLocalTime(&timeinfo))
    {
      rtc.adjust(DateTime(timeinfo.tm_year + 1900, timeinfo.tm_mon + 1,
                          timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min,
                          timeinfo.tm_sec));
      Serial.println("✓ RTC synchronized with NTP time");

      // Print updated RTC time
      DateTime now = rtc.now();
      Serial.print("Updated RTC Time: ");
      Serial.print(now.year());
      Serial.print("-");
      Serial.print(now.month());
      Serial.print("-");
      Serial.print(now.day());
      Serial.print(" ");
      Serial.print(now.hour());
      Serial.print(":");
      Serial.print(now.minute());
      Serial.print(":");
      Serial.println(now.second());
    }
    else
    {
      Serial.println("✗ Failed to get NTP time for RTC sync");
    }
  }
  else
  {
    Serial.println("WiFi not connected, cannot sync RTC with NTP");
  }
}

// Get the current RTC time
DateTime getCurrentRTC()
{
  if (!rtcAvailable)
  {
    // Return default time if RTC not available
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