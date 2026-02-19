#include "DHT11Sensor.h"
#include "LCDManager.h"

extern LiquidCrystal_I2C lcd; // Declare lcd as external
// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void initializeDHT()
{
  dht.begin();
}

void displayDHT11Data()
{
  float humidity = NAN;
  float temperature = NAN;

  int attempts = 0;
  // Read humidity and temperature
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  // Retry up to 5 times if the sensor fails to read
  while ((isnan(humidity) || isnan(temperature)) && attempts < 5)
  {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    attempts++;
    delay(1000);
  }

  // If sensor fails, show error message
  if (isnan(humidity) || isnan(temperature))
  {
    lcd.setCursor(0, 1);
    lcd.print("Err"); // Display error if sensor data is invalid
  }
  else
  {
    // Format temperature and humidity strings
    String tempStr = String(temperature, 1);
    String humidityStr = String(humidity, 1);

    // Display temperature with degree symbol
    lcd.print("                "); // Clear line
    lcd.setCursor(0, 1);
    lcd.print(tempStr);
    lcd.print((char)223); // Degree symbol
    lcd.print("C");

    // Display humidity with percentage symbol
    lcd.setCursor(8, 1);
    lcd.print(humidityStr);
    lcd.print("%");
    delay(1000);
  }
}