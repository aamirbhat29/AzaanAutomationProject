#include <TimeLib.h>
#include "LocationManager.h"
#include "TimeManager.h"
#include <Arduino.h>
#include "RCTManager.h"

String getCurrentDate()
{
  struct tm timeinfo;

  if (!timeSynced)
  {
    // Check if RTC is available before trying to use it
    if (isRTCAvailable())
    {
      // If time is not synchronized, fetch the date from RTC
      DateTime now = rtc.now();
      char dateStr[11]; // Format: YYYY-MM-DD
      snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", now.year(), now.month(), now.day());

      String currentDate = String(dateStr);
      Serial.println("Current date (from RTC): " + currentDate);
      return currentDate;
    }
    else
    {
      Serial.println("RTC not available and NTP not synced - cannot get date");
      return ""; // Return empty string
    }
  }

  // Get date from NTP time
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain date from NTP");

    // Try RTC as fallback
    if (isRTCAvailable())
    {
      DateTime now = rtc.now();
      char dateStr[11];
      snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", now.year(), now.month(), now.day());
      String currentDate = String(dateStr);
      Serial.println("Current date (from RTC fallback): " + currentDate);
      return currentDate;
    }

    return ""; // No date available
  }

  // Format the date string (e.g., "2025-01-27")
  char dateStr[11]; // Format: YYYY-MM-DD
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);

  String currentDate = String(dateStr);
  Serial.println("Current date (from NTP): " + currentDate);
  return currentDate;
}
