#include "LCDManager.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "TimeManager.h"
#include <Arduino.h>
#include <RCTManager.h>
#include "AzaanTimes.h"

// Define lcd here only
LiquidCrystal_I2C lcd(0x27, 16, 2);

void initializeLCD()
{
  Wire.begin(21, 22);
  lcd.begin(16, 2);
  lcd.backlight();
  lcd.clear();
}

void displayMessage(String message)
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(message);
}

extern String currentPrayerName[];
extern String prayerName;
extern String currentPrayerTimeToDisplay;

// Helper function to convert 24-hour time to 12-hour format
String convertTo12Hour(String time24)
{
  if (time24.length() < 5)
    return time24; // Safety check

  int hour = time24.substring(0, 2).toInt();
  String minute = time24.substring(3, 5);
  String period = "A";

  if (hour >= 12)
  {
    period = "P";
    if (hour > 12)
    {
      hour -= 12;
    }
  }
  else if (hour == 0)
  {
    hour = 12;
  }

  // Format without leading zero for single-digit hours
  String time12Hour = String(hour) + ":" + minute + period;
  return time12Hour;
}

// Main function to display time on the LCD
void displayTime(String currentTime)
{
  int hour = currentTime.substring(0, 2).toInt();
  String minute = currentTime.substring(3, 5);

  String period = "A";

  // Convert to 12-hour format
  if (hour >= 12)
  {
    period = "P";
    if (hour > 12)
    {
      hour -= 12;
    }
  }
  else if (hour == 0)
  {
    hour = 12;
  }

  // Format: "4:41A" (no leading zero)
  String time12Hour = String(hour) + ":" + minute + period;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(time12Hour); // Display current time

  // Display next prayer name and time (converted to 12-hour)
  if (prayerName.length() > 0 && currentPrayerTimeToDisplay.length() > 0)
  {
    lcd.setCursor(6, 0); // Start after time
    lcd.print(prayerName);

    // Convert prayer time to 12-hour format
    String prayerTime12 = convertTo12Hour(currentPrayerTimeToDisplay);

    lcd.setCursor(11, 0);
    lcd.print(prayerTime12);
  }
}