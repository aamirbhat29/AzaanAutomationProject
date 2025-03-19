#ifndef AZAAN_TIMES_H
#define AZAAN_TIMES_H

#include <Arduino.h> 

extern String fajrTime;
extern String otherPrayerTimes[4];
extern String ipstackKey;
extern String ipstackAPI;
extern String currentPrayerName[];
extern int lastDisplayedPrayerIndex;
extern bool prayerTimeDisplayed;
extern String currentPrayerTimeToDisplay;
extern String prayerName;
extern bool prayerDataUpdated;
extern bool maghribSequenceActive;
extern unsigned long duaStartTime;
extern bool duaPlayed;
extern bool showingLocation;
extern unsigned long locationStartTime;
void fetchPrayerTimes();
void playAzaan(int trackNumber);
void checkAndTriggerAzaan();
bool parsePrayerTimes(String responseBody);
void simulateButtonPress();
void fetchAzaanTimes();
void displayNextPrayerTime();
int getCurrentPrayerIndex();
String getCurrentPrayerName();
int timeToMinutes();

#endif
