#include "LCDManager.h"
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "TimeManager.h"
#include <Arduino.h>
#include <RCTManager.h>
#include "AzaanTimes.h"

LiquidCrystal_I2C lcd(0x27, 16, 2);

// Display state tracking
unsigned long lastRow1Update = 0;
unsigned long lastRow2Update = 0;
int row1Phase = 0; // 0 = time, 1 = DHT
int row2Phase = 0; // 0-3 = prayer rotation

// Store DHT values globally
float currentTemp = 0.0;
float currentHumidity = 0.0;

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
extern String fajrTime;
extern String otherPrayerTimes[];

// Update DHT values (called from main loop)
void updateDHTValues(float temp, float hum)
{
  currentTemp = temp;
  currentHumidity = hum;
}

// Convert 24-hour to 12-hour format
String convertTo12Hour(String time24)
{
  if (time24.length() < 5)
    return time24;

  int hour = time24.substring(0, 2).toInt();
  String minute = time24.substring(3, 5);
  String period = "AM";

  if (hour >= 12)
  {
    period = "PM";
    if (hour > 12)
    {
      hour -= 12;
    }
  }
  else if (hour == 0)
  {
    hour = 12;
  }

  // Format with space before AM/PM
  String time12Hour = String(hour) + ":" + minute + " " + period;
  return time12Hour;
}

// Update Row 1 (Time / DHT alternating)
void updateRow1(String currentTime)
{
  unsigned long currentMillis = millis();

  // Switch phase every 10 seconds (was 5)
  if (currentMillis - lastRow1Update >= 10000)
  {
    lastRow1Update = currentMillis;
    row1Phase = (row1Phase + 1) % 2;

    lcd.setCursor(0, 0);
    lcd.print("                "); // Clear row
    lcd.setCursor(0, 0);

    if (row1Phase == 0)
    {
      // Show time
      String time12Hour = convertTo12Hour(currentTime);
      lcd.print("Time: ");
      lcd.print(time12Hour);
    }
    else
    {
      // Show DHT with 1 decimal and degree symbol
      lcd.print("T:");
      lcd.print(currentTemp, 1);
      lcd.print((char)223); // Degree symbol
      lcd.print("C H:");
      lcd.print(currentHumidity, 0); // No decimal for humidity
      lcd.print("%");
    }
  }
}

// Update Row 2 (Prayer rotation)
void updateRow2()
{
  unsigned long currentMillis = millis();
  String currentTime = getCurrentTime();
  String currentTimeWithoutSeconds = currentTime.substring(0, 5);

  // Switch phase every 5 seconds
  if (currentMillis - lastRow2Update >= 5000)
  {
    lastRow2Update = currentMillis;
    row2Phase = (row2Phase + 1) % 4;

    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear row
    lcd.setCursor(0, 1);

    // Determine which prayers to show based on current time
    String prayers[4];
    String prayerTimes[4];
    int prayerCount = 0;

    // Build list of remaining prayers
    if (currentTimeWithoutSeconds < fajrTime)
    {
      // Before Fajr - show all 5 prayers
      prayers[0] = "Fajr";
      prayerTimes[0] = convertTo12Hour(fajrTime);
      prayers[1] = "Dhuhr";
      prayerTimes[1] = convertTo12Hour(otherPrayerTimes[0]);
      prayers[2] = "Asar";
      prayerTimes[2] = convertTo12Hour(otherPrayerTimes[1]);
      prayers[3] = "Magrib";
      prayerTimes[3] = convertTo12Hour(otherPrayerTimes[2]);
      prayerCount = 4;
    }
    else if (currentTimeWithoutSeconds < otherPrayerTimes[0])
    {
      // After Fajr, before Dhuhr
      prayers[0] = "Dhuhr";
      prayerTimes[0] = convertTo12Hour(otherPrayerTimes[0]);
      prayers[1] = "Asar";
      prayerTimes[1] = convertTo12Hour(otherPrayerTimes[1]);
      prayers[2] = "Magrib";
      prayerTimes[2] = convertTo12Hour(otherPrayerTimes[2]);
      prayers[3] = "Isha";
      prayerTimes[3] = convertTo12Hour(otherPrayerTimes[3]);
      prayerCount = 4;
    }
    else if (currentTimeWithoutSeconds < otherPrayerTimes[1])
    {
      // After Dhuhr, before Asar
      prayers[0] = "Asar";
      prayerTimes[0] = convertTo12Hour(otherPrayerTimes[1]);
      prayers[1] = "Magrib";
      prayerTimes[1] = convertTo12Hour(otherPrayerTimes[2]);
      prayers[2] = "Isha";
      prayerTimes[2] = convertTo12Hour(otherPrayerTimes[3]);
      prayerCount = 3;
      row2Phase = row2Phase % 3; // Only 3 prayers left
    }
    else if (currentTimeWithoutSeconds < otherPrayerTimes[2])
    {
      // After Asar, before Magrib
      prayers[0] = "Magrib";
      prayerTimes[0] = convertTo12Hour(otherPrayerTimes[2]);
      prayers[1] = "Isha";
      prayerTimes[1] = convertTo12Hour(otherPrayerTimes[3]);
      prayerCount = 2;
      row2Phase = row2Phase % 2; // Only 2 prayers left
    }
    else if (currentTimeWithoutSeconds < otherPrayerTimes[3])
    {
      // After Magrib, before Isha
      prayers[0] = "Isha";
      prayerTimes[0] = convertTo12Hour(otherPrayerTimes[3]);
      prayerCount = 1;
      row2Phase = 0; // Only 1 prayer left
    }
    else
    {
      // After Isha - show tomorrow's Fajr with arrow
      extern String nextDayFajrTime;
      if (nextDayFajrTime != "" && nextDayFajrTime != "05:48")
      {
        lcd.print((char)126); // Arrow
        lcd.print(" Tmrw Fajr ");
        lcd.print(convertTo12Hour(nextDayFajrTime));
      }
      else
      {
        lcd.print((char)126);
        lcd.print(" Tmrw Fajr Err");
      }
      return; // Don't rotate
    }

    // Display the current prayer in rotation with arrow symbol
    if (prayerCount > 0 && row2Phase < prayerCount)
    {
      lcd.print((char)126); // Arrow symbol: →
      lcd.print(" ");
      lcd.print(prayers[row2Phase]);
      lcd.print(" ");
      lcd.print(prayerTimes[row2Phase]);
    }
  }
}

// Main display function (called from loop)
void displayTime(String currentTime)
{
  updateRow1(currentTime);
  updateRow2();
}