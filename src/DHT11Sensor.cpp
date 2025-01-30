#include "DHT11Sensor.h"
#include "LCDManager.h"

extern LiquidCrystal_I2C lcd; // Declare lcd as external
// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);


void initializeDHT() {
  dht.begin();  // Initialize the sensor
}

void displayDHT11Data() {
  float humidity = NAN;
  float temperature = NAN;
  int attempts = 0;

  // Read humidity and temperature
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Retry up to 5 times if the sensor fails to read
  while ((isnan(humidity) || isnan(temperature)) && attempts < 5) {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    attempts++;
    delay(1000); // Retry after a short delay
  }

  // Clear the display to avoid overlap
  //lcd.clear();

  // If sensor fails, show error message
  if (isnan(humidity) || isnan(temperature)) {
    lcd.setCursor(0, 1);  // Set cursor to the beginning of the first row
    lcd.print("Err"); // Display error if sensor data is invalid
  } else {
    // Format temperature and humidity strings
    String tempStr = String(temperature, 1); // One decimal for temperature
    String humidityStr = String(humidity, 1); // One decimal for humidity

    // Display temperature with degree symbol
    lcd.print("                ");
    lcd.setCursor(0, 1);  // Set cursor to the first row
    lcd.print(tempStr); 
    lcd.print((char)223);  // Degree symbol
    lcd.print("C"); // Celsius unit

    // Display humidity with percentage symbol
    lcd.setCursor(8, 1);  // Set cursor to the second column of the first row
    lcd.print(humidityStr);
    lcd.print("%"); // Percentage symbol
    delay(2000);
  }
}