// AzaanTimes.h
#ifndef AZAANTIMES_H
#define AZAANTIMES_H

#include <Arduino.h>

// Declare external variables
extern String fajrTime;
extern String imsakTime;
extern String sunriseTime;
extern String otherPrayerTimes[];
extern String nextDayFajrTime;

// Tomorrow's prayer times (for after Isha)
extern String tomorrowImsakTime;
extern String tomorrowFajrTime;
extern String tomorrowSunriseTime;
extern String tomorrowPrayerTimes[];
extern String tomorrowHijriDate;
extern String tomorrowHijriMonth;

// Hijri date variables
extern String hijriDate;
extern String hijriMonth;
extern String hijriYear;
extern String gregorianDate;

extern String prayerName;
extern String currentPrayerTimeToDisplay;
extern String currentPrayerName[];

#define ADKEY1_PIN 13

// Function declarations
void fetchPrayerTimes();
void fetchAzaanTimes();
void fetchTomorrowFajr();
bool parsePrayerTimes(String responseBody);
void checkAndTriggerAzaan();
void simulateButtonPress();
void sendTelegramPrayerTimes();
String addMinutesToTime(String timeStr, int minutesToAdd);
String convertTo12HourFormat(String time24);

#endif // AZAANTIMES_H