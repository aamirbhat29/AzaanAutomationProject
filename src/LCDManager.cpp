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
  Wire.begin(21, 22); // Make sure your SDA/SCL pins are correctly set
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

// Main function to display time on the LCD
void displayTime(String currentTime)
{
  int hour = currentTime.substring(0, 2).toInt();
  String minute = currentTime.substring(3, 5);

  String period = "A"; // Changed: Single letter

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

  // Format: "4:41A" (no colon between minute and period)
  String time12Hour = String(hour) + ":" + minute + period;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(time12Hour); // e.g., "4:41A"

  // Display next prayer name and time
  if (prayerName.length() > 0 && currentPrayerTimeToDisplay.length() > 0)
  {
    lcd.setCursor(6, 0);   // Start after "4:41A " (position 6)
    lcd.print(prayerName); // "Fajr"
    lcd.setCursor(11, 0);  // After "Fajr " (position 11)

    // Show only HH:MM
    if (currentPrayerTimeToDisplay.length() > 5)
    {
      lcd.print(currentPrayerTimeToDisplay.substring(0, 5));
    }
    else
    {
      lcd.print(currentPrayerTimeToDisplay);
    }
  }
}
