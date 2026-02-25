#include "DHT11Sensor.h"
#include "LCDManager.h"

extern LiquidCrystal_I2C lcd;
extern void updateDHTValues(float temp, float hum);

DHT dht(DHTPIN, DHTTYPE);

bool dhtWorking = false;

void initializeDHT()
{
  dht.begin();

  float testRead = dht.readTemperature();
  if (!isnan(testRead))
  {
    dhtWorking = true;
    Serial.println("✓ DHT11 sensor detected and working");
  }
  else
  {
    dhtWorking = false;
    Serial.println("⚠ DHT11 sensor not detected");
  }
}

void displayDHT11Data()
{
  if (!dhtWorking)
  {
    return;
  }

  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  int attempts = 0;
  while ((isnan(humidity) || isnan(temperature)) && attempts < 3)
  {
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();
    attempts++;
    delay(500);
  }

  if (isnan(humidity) || isnan(temperature))
  {
    dhtWorking = false;
    return;
  }
  else
  {
    // Pass values to LCD manager for display
    updateDHTValues(temperature, humidity);
  }
}