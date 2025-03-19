#include "LCDManager.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "TimeManager.h"
#include <Arduino.h>
#include <RCTManager.h>
#include "AzaanTimes.h"

// Define LCD instance
LiquidCrystal_I2C lcd(0x27, 16, 2);

// External variables
extern String currentPrayerTimeToDisplay;
extern String prayerName;
extern bool timeSynced;
extern bool prayerDataUpdated;

// Initialize LCD
void initializeLCD() {
    Wire.begin(22, 21);  // Ensure correct SDA/SCL pins
    lcd.begin(16, 2);
    lcd.backlight();
    lcd.clear();
}

void displayMessage(String message) {
  //lcd.clear();  // Clear the screen before displaying a new message
  lcd.setCursor(0, 0);
  lcd.print(message);
}

// Modified helper function to convert 24-hour time to 12-hour format with optional period
String convertTo12HourFormat(String time24, bool includePeriod = true) {
    int hour = time24.substring(0, 2).toInt();
    String minute = time24.substring(3, 5);
    String period = "AM";
    
    if (hour >= 12) {
        period = "PM";
        if (hour > 12) {
            hour -= 12;
        }
    } else if (hour == 0) {
        hour = 12;
    }
    
    String result = String(hour) + ":" + minute;
    if (includePeriod) {
        result += " " + period;
    }
    return result;
}

// Function to display time and prayer information
void displayTime(String currentTime) {
    String time12Hour = convertTo12HourFormat(currentTime, false); // Includes period
    
    lcd.clear();
    lcd.setCursor(0, 0);
    // if (!timeSynced) {
    //     lcd.print("RTC: ");
    // } else {
    //     lcd.print("Time: ");
    // }
    // lcd.clear();
    lcd.print(time12Hour);
    
    lcd.setCursor(6, 0);
    // Convert prayer time to 12-hour format without period
    //Serial.println("hello " + currentPrayerTimeToDisplay);
    String prayerTime12Hour = convertTo12HourFormat(currentPrayerTimeToDisplay, false);
    lcd.print(prayerName + " " + prayerTime12Hour);
    
    // Debug output
    //Serial.println("Time: " + time12Hour);
    //Serial.println("Prayer: " + prayerName + " " + prayerTime12Hour);
    
    prayerDataUpdated = false; // Reset update flag
}