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

// FIXED: Changed to Srinagar (you're in Kashmir!)
String location = "Srinagar%2C%20India"; // Was: Delhi%2C%20India

bool prayerTimesFetched = false;
bool prayerTimesUpdateAttempted = false;
int fetchRetryCount = 0;

void fetchPrayerTimes()
{
  Serial.println("Fetching Azaan times from API...");
  delay(1000);
  fetchAzaanTimes();
  // Print updated times
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

    // FIXED: Reduced timeout from 10 seconds to 3 seconds
    const unsigned long timeout = 3000; // 3 seconds timeout

    // Retry logic with timeout
    while (currentDate == "" && millis() - startTime < timeout)
    {
      currentDate = getCurrentDate();
      if (currentDate != "")
        break;
      delay(300); // Wait 300ms before retrying (was 500ms)
    }

    if (currentDate == "")
    {
      Serial.println("Unable to fetch current date after timeout. Using RTC fallback...");
      // Try one more time with RTC directly
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

    // FIXED: Changed timezone from UTC to Asia/Kolkata (IST)
    String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/" + formattedDate +
                     "?address=" + location +
                     "&x7xapikey=" + azaanTimeAPIKey +
                     "&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6" +
                     "&school=1&midnightMode=0" +
                     "&timezonestring=Asia/Kolkata" + // CHANGED: Was UTC!
                     "&calendarMethod=UAQ";

    Serial.println("Current Date: " + currentDate);
    Serial.println("Constructed API URL: " + curlAPI);

    HTTPClient http;
    http.setTimeout(5000); // ADDED: 5 second HTTP timeout
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

  // Parse and update prayer times
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

  // IMPROVED: Fetch prayer times at 2:00 AM with better error handling
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
        delay(2000); // Wait 2 seconds before retrying (was 5)
      }
    }

    if (!prayerTimesFetched)
    {
      Serial.println("✗ Failed after 3 attempts. Using existing times.");
    }

    prayerTimesUpdateAttempted = true;
  }

  // Reset the flag after 2:01 AM
  if (currentTimeWithoutSeconds == "02:01")
  {
    prayerTimesUpdateAttempted = false;
  }

  // Determine current/next prayer
  if (currentTimeWithoutSeconds < fajrTime)
  {
    prayerName = currentPrayerName[0];
    currentPrayerTimeToDisplay = fajrTime;
  }
  else if (currentTimeWithoutSeconds <= otherPrayerTimes[0] && currentTimeWithoutSeconds > fajrTime)
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
  else if (currentTimeWithoutSeconds > otherPrayerTimes[2] && currentTimeWithoutSeconds < "23:59")
  {
    prayerName = currentPrayerName[4];
    currentPrayerTimeToDisplay = otherPrayerTimes[3];
  }

  // Check if it's time for a prayer
  if (currentTimeWithoutSeconds == fajrTime)
  {
    Serial.println("Time for Fajr Azaan!");
    playAzaan(1);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[0])
  {
    Serial.println("Time for Dhuhr Azaan!");
    playAzaan(2);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[1])
  {
    Serial.println("Time for Asr Azaan!");
    playAzaan(2);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[2])
  {
    Serial.println("Time for Maghrib Azaan!");
    playAzaan(2);
  }
  else if (currentTimeWithoutSeconds == otherPrayerTimes[3])
  {
    Serial.println("Time for Isha Azaan!");
    playAzaan(2);
  }
}
