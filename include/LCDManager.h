#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Arduino.h> 

void initializeLCD();
void displayMessage(String message);
void displayTime(String currentTime);
void displayPrayerTime(int prayerIndex);
void displayPT(String pT);


#endif
