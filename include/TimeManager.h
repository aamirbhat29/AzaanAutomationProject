#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <TimeLib.h>
#include <Arduino.h> 

extern bool timeSynced;  // Flag for time synchronization status
bool getNtpTime(const char* server);
void syncTimeWithMultipleServers();
void setTime(int hours, int minutes, int seconds, int day, int month, int year);
String getCurrentTime();
void handleTimeSync();

#endif
