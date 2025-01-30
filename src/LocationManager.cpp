#include <TimeLib.h>  // Include this for date/time functions
#include "LocationManager.h"
#include "TimeManager.h"
#include <Arduino.h>  // This ensures the String class is available
#include "RCTManager.h"

String getCurrentDate() {
  struct tm timeinfo;

  if (!timeSynced) {
    // If time is not synchronized, fetch the date from RTC
    DateTime now = rtc.now();  // Adjust this line based on your RTC library
    char dateStr[11];  // Format: YYYY-MM-DD
    snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", now.year(), now.month(), now.day());

    String currentDate = String(dateStr);
    Serial.println("Current date (from RTC): " + currentDate);
    return currentDate;
  }

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain date from NTP");
    return "Error: Unable to get date";
  }

  // Format the date string (e.g., "2025-01-27")
  char dateStr[11];  // Format: YYYY-MM-DD
  strftime(dateStr, sizeof(dateStr), "%Y-%m-%d", &timeinfo);

  String currentDate = String(dateStr);
  Serial.println("Current date (from NTP): " + currentDate);
  return currentDate;
}
