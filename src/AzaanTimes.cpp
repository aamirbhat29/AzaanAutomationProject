#include "AzaanTimes.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LocationManager.h>
#include "DFPlayerManager.h"
#include "TimeManager.h"
#include <Arduino.h>
#include "LCDManager.h"
#include "RCTManager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>

// ========== TELEGRAM CONFIGURATION ==========
String telegramBotToken = "8773448920:AAGTwSgxNuoORHjzGa3XzpjW8Q9u_0B2xqg";
String telegramChatID = "1787825575";

// Global variables
String fajrTime = "05:48";
String imsakTime = "05:36";
String sunriseTime = "07:06";
String otherPrayerTimes[] = {"12:45", "16:30", "18:00", "20:31"};

// Tomorrow's prayer times (for display after Isha)
String tomorrowImsakTime = "05:36";
String tomorrowFajrTime = "05:48";
String tomorrowSunriseTime = "07:06";
String tomorrowPrayerTimes[] = {"12:45", "16:30", "18:00", "20:31"};
String tomorrowHijriDate = "";
String tomorrowHijriMonth = "";

String ipstackKey = "fd070d6f9a4c085dfe619ce5f12a19c1";
String ipstackAPI = "http://api.ipstack.com/check?access_key=" + ipstackKey;
String azaanTimeAPIKey = "b91f004f7f391b4f380805620a179d44";
String location = "Srinagar%2C%20India";

// Hijri date variables
String hijriDate = "";
String hijriMonth = "";
String hijriYear = "";
String gregorianDate = "";

String nextDayFajrTime = "05:48";

bool prayerTimesFetched = false;
bool prayerTimesUpdateAttempted = false;
bool telegramMessageSent = false;
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

// Convert 24h to 12h format
String convertTo12HourFormat(String time24)
{
  int hour = time24.substring(0, 2).toInt();
  String minute = time24.substring(3, 5);
  String period = "AM";

  if (hour >= 12)
  {
    period = "PM";
    if (hour > 12)
      hour -= 12;
  }
  else if (hour == 0)
  {
    hour = 12;
  }

  return String(hour) + ":" + minute + " " + period;
}

// Send prayer times via Telegram
void sendTelegramPrayerTimes()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi not connected, cannot send Telegram message");
    return;
  }

  Serial.println("Sending Telegram prayer times message...");

  // Build message (plain text with emojis)
  String message = "🕌 RAMADAN PRAYER TIMES 🕌\n\n";
  message += "📅 " + hijriDate + " " + hijriMonth + " " + hijriYear + " AH\n";
  message += "📅 " + gregorianDate + "\n";
  message += "━━━━━━━━━━━━━━━━━━━━\n\n";

  message += "🌙 SEHRI (Imsak): " + convertTo12HourFormat(imsakTime) + "\n";
  message += "   (Stop eating before Fajr)\n\n";

  message += "🌅 FAJR: " + convertTo12HourFormat(fajrTime) + "\n";
  message += "☀️ SUNRISE: " + convertTo12HourFormat(sunriseTime) + "\n";
  message += "🌞 DHUHR: " + convertTo12HourFormat(otherPrayerTimes[0]) + "\n";
  message += "🌤️ ASR: " + convertTo12HourFormat(otherPrayerTimes[1]) + "\n";
  message += "🌆 MAGHRIB (Iftar): " + convertTo12HourFormat(otherPrayerTimes[2]) + "\n";
  message += "🌙 ISHA: " + convertTo12HourFormat(otherPrayerTimes[3]) + "\n\n";

  message += "━━━━━━━━━━━━━━━━━━━━\n";
  message += "📍 Srinagar, Kashmir\n";
  message += "🤲 Ramadan Mubarak!\n";
  message += "━━━━━━━━━━━━━━━━━━━━\n\n";
  message += "⚙️ Automated by ESP32 Azaan System";

  // URL encode
  message.replace("\n", "%0A");
  message.replace(" ", "%20");
  message.replace(":", "%3A");
  message.replace("(", "%28");
  message.replace(")", "%29");
  message.replace("!", "%21");
  message.replace(",", "%2C");

  String url = "https://api.telegram.org/bot" + telegramBotToken +
               "/sendMessage?chat_id=" + telegramChatID +
               "&text=" + message;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient https;
  https.begin(client, url);
  https.setTimeout(10000);

  int httpResponseCode = https.GET();

  if (httpResponseCode == 200)
  {
    Serial.println("✓ Telegram message sent successfully!");
    telegramMessageSent = true;
  }
  else
  {
    Serial.print("✗ Telegram message failed. HTTP code: ");
    Serial.println(httpResponseCode);
  }

  https.end();
}

void fetchPrayerTimes()
{
  Serial.println("Fetching Azaan times from API...");
  delay(1000);
  fetchAzaanTimes();
  Serial.println("Azaan times updated:");
  Serial.println("Imsak: " + imsakTime);
  Serial.println("Fajr: " + fajrTime);
  Serial.println("Sunrise: " + sunriseTime);
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

    // Timezone: Asia/Kolkata (IST)
    // Tune: Maghrib +3 min to match mosque practice
    String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/" + formattedDate +
                     "?address=" + location +
                     "&x7xapikey=" + azaanTimeAPIKey +
                     "&method=3&shafaq=general&tune=11%2C0%2C0%2C0%2C0%2C3%2C0%2C4%2C-6" +
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

        // Also fetch tomorrow's complete schedule
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

// Fetch tomorrow's complete prayer schedule
void fetchTomorrowFajr()
{
  if ((WiFi.status() != WL_CONNECTED))
  {
    Serial.println("WiFi not connected, cannot fetch tomorrow's prayers");
    nextDayFajrTime = "";
    return;
  }

  Serial.println("Fetching tomorrow's complete prayer schedule...");

  char tomorrowDate[11];

  // Try RTC first
  if (isRTCAvailable())
  {
    Serial.println("Using RTC for tomorrow's date calculation");
    DateTime now = rtc.now();
    DateTime tomorrow = DateTime(now.unixtime() + 86400);
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
      nextDayFajrTime = "";
      return;
    }

    time_t now = mktime(&timeinfo);
    now += 86400;
    struct tm *tomorrow = localtime(&now);

    snprintf(tomorrowDate, sizeof(tomorrowDate), "%02d-%02d-%04d",
             tomorrow->tm_mday, tomorrow->tm_mon + 1, tomorrow->tm_year + 1900);
  }

  String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/" + String(tomorrowDate) +
                   "?address=" + location +
                   "&x7xapikey=" + azaanTimeAPIKey +
                   "&method=3&shafaq=general&tune=11%2C0%2C0%2C0%2C0%2C3%2C0%2C4%2C-6" +
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
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, responseBody);

    if (!error)
    {
      // Parse ALL tomorrow's prayer times
      tomorrowImsakTime = doc["data"]["timings"]["Imsak"].as<String>();
      tomorrowFajrTime = doc["data"]["timings"]["Fajr"].as<String>();
      tomorrowSunriseTime = doc["data"]["timings"]["Sunrise"].as<String>();
      tomorrowPrayerTimes[0] = doc["data"]["timings"]["Dhuhr"].as<String>();
      tomorrowPrayerTimes[1] = doc["data"]["timings"]["Asr"].as<String>();
      tomorrowPrayerTimes[2] = doc["data"]["timings"]["Maghrib"].as<String>();
      tomorrowPrayerTimes[3] = doc["data"]["timings"]["Isha"].as<String>();

      tomorrowHijriDate = doc["data"]["date"]["hijri"]["day"].as<String>();
      tomorrowHijriMonth = doc["data"]["date"]["hijri"]["month"]["en"].as<String>();

      nextDayFajrTime = tomorrowFajrTime;

      Serial.println("✓ Tomorrow's complete schedule fetched");
    }
    else
    {
      Serial.println("✗ Failed to parse tomorrow's prayers JSON");
      nextDayFajrTime = "";
    }
  }
  else
  {
    Serial.print("✗ Failed to fetch tomorrow's prayers. HTTP code: ");
    Serial.println(httpResponseCode);
    nextDayFajrTime = "";
  }

  http.end();
}

bool parsePrayerTimes(String responseBody)
{
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, responseBody);

  if (error)
  {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.f_str());
    return false;
  }

  // Parse prayer times
  imsakTime = doc["data"]["timings"]["Imsak"].as<String>();
  fajrTime = doc["data"]["timings"]["Fajr"].as<String>();
  sunriseTime = doc["data"]["timings"]["Sunrise"].as<String>();
  otherPrayerTimes[0] = doc["data"]["timings"]["Dhuhr"].as<String>();
  otherPrayerTimes[1] = doc["data"]["timings"]["Asr"].as<String>();
  otherPrayerTimes[2] = doc["data"]["timings"]["Maghrib"].as<String>();
  otherPrayerTimes[3] = doc["data"]["timings"]["Isha"].as<String>();

  // Parse Hijri date
  hijriDate = doc["data"]["date"]["hijri"]["day"].as<String>();
  hijriMonth = doc["data"]["date"]["hijri"]["month"]["en"].as<String>();
  hijriYear = doc["data"]["date"]["hijri"]["year"].as<String>();

  // Parse Gregorian date
  String gregDay = doc["data"]["date"]["gregorian"]["day"].as<String>();
  String gregMonth = doc["data"]["date"]["gregorian"]["month"]["en"].as<String>();
  String gregYear = doc["data"]["date"]["gregorian"]["year"].as<String>();
  gregorianDate = gregDay + " " + gregMonth + " " + gregYear;

  Serial.println("Hijri Date: " + hijriDate + " " + hijriMonth + " " + hijriYear);
  Serial.println("Gregorian Date: " + gregorianDate);

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

  // Send Telegram message at 12:01 AM (start of new day)
  if (currentTimeWithoutSeconds == "00:01" && !telegramMessageSent)
  {
    Serial.println("=== Sending Daily Telegram Message (12:01 AM) ===");
    sendTelegramPrayerTimes();
    telegramMessageSent = true;
  }

  if (currentTimeWithoutSeconds == "02:01")
  {
    prayerTimesUpdateAttempted = false;
  }

  if (currentTimeWithoutSeconds == "00:02")
  {
    telegramMessageSent = false; // Reset for next day
  }

  // Determine current/next prayer for display
  if (currentTimeWithoutSeconds < fajrTime)
  {
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
    // AFTER Isha - switch to TOMORROW's full schedule
    Serial.println("After Isha - switching to tomorrow's prayer schedule");

    if (nextDayFajrTime != "" && nextDayFajrTime != "05:48")
    {
      // Load tomorrow's prayers into display variables
      fajrTime = tomorrowFajrTime;
      otherPrayerTimes[0] = tomorrowPrayerTimes[0];
      otherPrayerTimes[1] = tomorrowPrayerTimes[1];
      otherPrayerTimes[2] = tomorrowPrayerTimes[2];
      otherPrayerTimes[3] = tomorrowPrayerTimes[3];

      prayerName = currentPrayerName[0];
      currentPrayerTimeToDisplay = tomorrowFajrTime;

      Serial.println("Tomorrow's schedule loaded for display");
    }
    else
    {
      prayerName = "Err";
      currentPrayerTimeToDisplay = "404";
      Serial.println("ERROR: Could not fetch tomorrow's prayers");
    }
  }

  // Calculate Maghrib azaan time (1 min after Maghrib)
  String maghribAzaanTime = addMinutesToTime(otherPrayerTimes[2], 1);

  // Check if it's time for prayers
  if (currentTimeWithoutSeconds == fajrTime)
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