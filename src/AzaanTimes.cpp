#include "AzaanTimes.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LocationManager.h>
#include "DFPlayerManager.h"
#include "TimeManager.h"
#include <Arduino.h>
#include "LCDManager.h"
#include "RCTManager.h"

// Global variables
String fajrTime = "05:48";
String otherPrayerTimes[] = {"12:45", "16:30", "18:00", "20:31"};
String ipstackKey = "fd070d6f9a4c085dfe619ce5f12a19c1";
String ipstackAPI = "http://api.ipstack.com/check?access_key=" + ipstackKey;
String azaanTimeAPIKey = "b91f004f7f391b4f380805620a179d44";
String location = "Srinagar%2C%20India";

// Store next day's Fajr time
String nextDayFajrTime = "05:48"; // Will be updated

bool prayerTimesFetched = false;
bool prayerTimesUpdateAttempted = false;
int fetchRetryCount = 0;

// Helper function to add minutes to a time string
String addMinutesToTime(String timeStr, int minutesToAdd)
{
  int hour = timeStr.substring(0, 2).toInt();
  int minute = timeStr.substring(3, 5).toInt();

  minute += minutesToAdd;

  while (minute >= 60)
  {
    minute -= 60;
    hour += 1;
  }
  while (minute < 0)
  {
    minute += 60;
    hour -= 1;
  }

  if (hour >= 24)
    hour -= 24;
  if (hour < 0)
    hour += 24;

  String result = (hour < 10 ? "0" : "") + String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute);
  return result;
}

void fetchPrayerTimes()
{
  Serial.println("Fetching Azaan times from API...");
  delay(1000);
  fetchAzaanTimes();
  Serial.println("Azaan times updated:");
  Serial.println("Fajr: " + fajrTime);
  for (int i = 0; i < 4; i++)
  {
    Serial.println("Prayer " + String(i + 2) + ": " + otherPrayerTimes[i]);
  }
}

void fetchAzaanTimes()
{
  if ((WiFi.status() == WL_CONNECTED))
  {
    String currentDate;
    String loc;
    unsigned long startTime = millis();
    const unsigned long timeout = 3000;

    while (currentDate == "" && millis() - startTime < timeout)
    {
      currentDate = getCurrentDate();
      if (currentDate != "")
        break;
      delay(300);
    }

    if (currentDate == "")
    {
      Serial.println("Unable to fetch current date after timeout. Using RTC fallback...");
      DateTime now = rtc.now();
      char dateStr[11];
      snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", now.year(), now.month(), now.day());
      currentDate = String(dateStr);

      if (currentDate == "")
      {
        Serial.println("Critical: Cannot get date. Aborting API call...");
        return;
      }
    }

    String formattedDate = currentDate.substring(8, 10) + "-" + currentDate.substring(5, 7) + "-" + currentDate.substring(0, 4);

    String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/" + formattedDate +
                     "?address=" + location +
                     "&x7xapikey=" + azaanTimeAPIKey +
                     "&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6" +
                     "&school=1&midnightMode=0" +
                     "&timezonestring=Asia/Kolkata" +
                     "&calendarMethod=UAQ";

    Serial.println("Current Date: " + currentDate);
    Serial.println("Constructed API URL: " + curlAPI);

    HTTPClient http;
    http.setTimeout(5000);
    http.begin(curlAPI);

    int httpResponseCode = http.GET();

    if (httpResponseCode == 200)
    {
      String responseBody = http.getString();
      Serial.println("API Response:");
      Serial.println(responseBody);

      bool success = parsePrayerTimes(responseBody);

      if (success)
      {
        prayerTimesFetched = true;
        Serial.println("✓ Prayer times successfully fetched and parsed!");

        // Also fetch tomorrow's Fajr time for display after Isha
        fetchTomorrowFajr();
      }
      else
      {
        Serial.println("✗ Failed to parse Azaan times from response.");
      }
    }
    else
    {
      Serial.print("HTTP request failed. Code: ");
      Serial.println(httpResponseCode);
      Serial.println("Using fallback prayer times.");
    }
    http.end();
  }
  else
  {
    Serial.println("WiFi is disconnected. Cannot fetch prayer times. Using offline times.");
  }
}

// NEW: Fetch tomorrow's Fajr time
void fetchTomorrowFajr()
{
  if ((WiFi.status() != WL_CONNECTED))
  {
    Serial.println("WiFi not connected, cannot fetch tomorrow's Fajr");
    nextDayFajrTime = ""; // Mark as failed
    return;
  }

  Serial.println("Fetching tomorrow's Fajr time...");

  char tomorrowDate[11];

  // Try RTC first
  if (isRTCAvailable())
  {
    Serial.println("Using RTC for tomorrow's date calculation");
    DateTime now = rtc.now();
    DateTime tomorrow = DateTime(now.unixtime() + 86400); // Add 86400 seconds (1 day)
    snprintf(tomorrowDate, sizeof(tomorrowDate), "%02d-%02d-%04d",
             tomorrow.day(), tomorrow.month(), tomorrow.year());
  }
  // Fallback to NTP
  else
  {
    Serial.println("RTC not available, using NTP for tomorrow's date");
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to get time from NTP for tomorrow's date");
      nextDayFajrTime = ""; // Mark as failed
      return;
    }

    // Add 1 day to current time
    time_t now = mktime(&timeinfo);
    now += 86400; // Add 1 day in seconds
    struct tm *tomorrow = localtime(&now);

    snprintf(tomorrowDate, sizeof(tomorrowDate), "%02d-%02d-%04d",
             tomorrow->tm_mday, tomorrow->tm_mon + 1, tomorrow->tm_year + 1900);
  }

  String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/" + String(tomorrowDate) +
                   "?address=" + location +
                   "&x7xapikey=" + azaanTimeAPIKey +
                   "&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6" +
                   "&school=1&midnightMode=0" +
                   "&timezonestring=Asia/Kolkata" +
                   "&calendarMethod=UAQ";

  HTTPClient http;
  http.setTimeout(5000);
  http.begin(curlAPI);

  int httpResponseCode = http.GET();

  if (httpResponseCode == 200)
  {
    String responseBody = http.getString();
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, responseBody);

    if (!error)
    {
      nextDayFajrTime = doc["data"]["timings"]["Fajr"].as<String>();
      Serial.print("✓ Tomorrow's Fajr: ");
      Serial.println(nextDayFajrTime);
    }
    else
    {
      Serial.println("✗ Failed to parse tomorrow's Fajr JSON");
      nextDayFajrTime = ""; // Mark as failed
    }
  }
  else
  {
    Serial.print("✗ Failed to fetch tomorrow's Fajr. HTTP code: ");
    Serial.println(httpResponseCode);
    nextDayFajrTime = ""; // Mark as failed
  }

  http.end();
}

bool parsePrayerTimes(String responseBody)
{
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, responseBody);

  if (error)
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.f_str());
    return false;
  }

  fajrTime = doc["data"]["timings"]["Fajr"].as<String>();
  otherPrayerTimes[0] = doc["data"]["timings"]["Dhuhr"].as<String>();
  otherPrayerTimes[1] = doc["data"]["timings"]["Asr"].as<String>();
  otherPrayerTimes[2] = doc["data"]["timings"]["Maghrib"].as<String>();
  otherPrayerTimes[3] = doc["data"]["timings"]["Isha"].as<String>();

  return true;
}

void simulateButtonPress()
{
  pinMode(ADKEY1_PIN, OUTPUT);
  digitalWrite(ADKEY1_PIN, LOW);
  delay(100);
  digitalWrite(ADKEY1_PIN, HIGH);
  pinMode(ADKEY1_PIN, INPUT_PULLUP);
}

String currentPrayerTime[] = {fajrTime, otherPrayerTimes[0], otherPrayerTimes[1], otherPrayerTimes[2], otherPrayerTimes[3]};
int lastDisplayedPrayerIndex = -1;
bool prayerTimeDisplayed = false;

String currentPrayerName[] = {"Fajr", "Dhuhr", "Asr", "Magrib", "Isha"};
String prayerName;
String currentPrayerTimeToDisplay;

void checkAndTriggerAzaan()
{
  String currentTime = getCurrentTime();
  String currentTimeWithoutSeconds = currentTime.substring(0, 5);

  // Fetch prayer times at 2:00 AM
  if (currentTimeWithoutSeconds == "02:00" && !prayerTimesUpdateAttempted)
  {
    Serial.println("=== Daily Prayer Times Update (2:00 AM) ===");
    fetchRetryCount = 0;

    while (!prayerTimesFetched && fetchRetryCount < 3)
    {
      fetchPrayerTimes();
      fetchRetryCount++;

      if (prayerTimesFetched)
      {
        Serial.println("✓ Prayer times successfully updated!");
        break;
      }
      else
      {
        Serial.print("Retry ");
        Serial.print(fetchRetryCount);
        Serial.println("/3 to fetch prayer times...");
        delay(2000);
      }
    }

    if (!prayerTimesFetched)
    {
      Serial.println("✗ Failed after 3 attempts. Using existing times.");
    }

    prayerTimesUpdateAttempted = true;
  }

  if (currentTimeWithoutSeconds == "02:01")
  {
    prayerTimesUpdateAttempted = false;
  }

  // Determine current/next prayer for display
  if (currentTimeWithoutSeconds < fajrTime)
  {
    // Before today's Fajr - show today's Fajr
    prayerName = currentPrayerName[0];
    currentPrayerTimeToDisplay = fajrTime;
  }
  else if (currentTimeWithoutSeconds <= otherPrayerTimes[0] && currentTimeWithoutSeconds >= fajrTime)
  {
    prayerName = currentPrayerName[1];
    currentPrayerTimeToDisplay = otherPrayerTimes[0];
  }
  else if (currentTimeWithoutSeconds <= otherPrayerTimes[1] && currentTimeWithoutSeconds > otherPrayerTimes[0])
  {
    prayerName = currentPrayerName[2];
    currentPrayerTimeToDisplay = otherPrayerTimes[1];
  }
  else if (currentTimeWithoutSeconds <= otherPrayerTimes[2] && currentTimeWithoutSeconds > otherPrayerTimes[1])
  {
    prayerName = currentPrayerName[3];
    currentPrayerTimeToDisplay = otherPrayerTimes[2];
  }
  else if (currentTimeWithoutSeconds <= otherPrayerTimes[3] && currentTimeWithoutSeconds > otherPrayerTimes[2])
  {
    prayerName = currentPrayerName[4];
    currentPrayerTimeToDisplay = otherPrayerTimes[3];
  }
  else
  {
    // AFTER Isha (past 19:41) until midnight - show TOMORROW's Fajr
    prayerName = currentPrayerName[0];

    // Use tomorrow's Fajr if we have it
    if (nextDayFajrTime != "05:48" && nextDayFajrTime != "")
    {
      currentPrayerTimeToDisplay = nextDayFajrTime;
    }
    else
    {
      // ERROR: Could not fetch tomorrow's Fajr (WiFi/RTC both failed)
      prayerName = "Err";
      currentPrayerTimeToDisplay = "404";
      Serial.println("ERROR: Could not fetch tomorrow's Fajr - showing error on display");
    }
  }

  // Calculate times for special prayers
  String suhoorTime = addMinutesToTime(fajrTime, -50);
  String maghribAzaanTime = addMinutesToTime(otherPrayerTimes[2], 1);

  // Check if it's time for Suhoor alarm or prayers
  if (currentTimeWithoutSeconds == suhoorTime)
  {
    Serial.println("=== TIME FOR SUHOOR ALARM ===");
    playAzaan(7);
  }
  else if (currentTimeWithoutSeconds == fajrTime)
  {
    Serial.println("=== TIME FOR FAJR AZAAN ===");
    playAzaan(1);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[0])
  {
    Serial.println("=== TIME FOR DHUHR AZAAN ===");
    playAzaan(2);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[1])
  {
    Serial.println("=== TIME FOR ASR AZAAN ===");
    playAzaan(3);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[2])
  {
    Serial.println("=== TIME FOR MAGHRIB - IFTAR ===");
    playAzaan(4);
  }
  else if (currentTimeWithoutSeconds == maghribAzaanTime)
  {
    Serial.println("=== TIME FOR MAGHRIB AZAAN ===");
    playAzaan(5);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[3])
  {
    Serial.println("=== TIME FOR ISHA AZAAN ===");
    playAzaan(6);
  }
}