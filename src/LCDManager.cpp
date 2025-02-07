#include "LCDManager.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "TimeManager.h"
#include <Arduino.h>
#include <RCTManager.h>
#include "AzaanTimes.h"


// Define lcd here only
LiquidCrystal_I2C lcd(0x27, 16, 2);

void initializeLCD() {
  Wire.begin(22, 21);  // Make sure your SDA/SCL pins are correctly set
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
}

void displayMessage(String message) {
  //lcd.clear();  // Clear the screen before displaying a new message
  lcd.setCursor(0, 0);
  lcd.print(message);
}

 extern String currentPrayerName[];
 extern String prayerName;
// Main function to display time on the LCD
void displayTime(String currentTime) {
  int hour = currentTime.substring(0, 2).toInt();  // Extract the hour part
  String minute = currentTime.substring(3, 5);     // Extract the minute part
  
  String period = "AM";
  
  // Convert to 12-hour format
  if (hour >= 12) {
    period = "PM";
    if (hour > 12) {
      hour -= 12;  // Convert hour to 12-hour format
    }
  } else if (hour == 0) {
    hour = 12; // Handle midnight (00:00) case
  }

  // Format the hour and minute back into a string with the period
  String time12Hour = String(hour) + ":" + minute + period;

  lcd.clear();  // Clear the screen before displaying time
  lcd.setCursor(0, 0);
  if (!timeSynced) {
    lcd.print("RTC: ");  // Indicate RTC time is being used
  } else {
    lcd.print("Time: ");  // Indicate synced time is being used
  }
  lcd.clear();  // Clear the screen before displaying time
  lcd.print(time12Hour);  // Display the current time
  lcd.setCursor(8, 0);
  lcd.print(prayerName);
  //Serial.println(time12Hour);
  //Serial.println(prayerName);
  lcd.print(currentPrayerTimeToDisplay); 
  //Serial.println(currentPrayerTimeToDisplay);
  prayerDataUpdated = false; // Reset the flag after displaying
  // Serial.println("Displaying prayerName: " + prayerName);
  // Serial.println("Displaying currentPrayerTimeToDisplay: " + currentPrayerTimeToDisplay);
}


// void displayPT(String pT) {
//    // Check if the current prayer is already displayed
//     // Update the index of the last displayed prayer
//     //lastDisplayedPrayerIndex = prayerIndex;

//     // Clear the LCD and display the current prayer name and time
//     //lcd.clear();
//     lcd.setCursor(9, 0);
//     lcd.print("ASR");
//     lcd.setCursor(11, 0);
//     lcd.print(pT);
// }



