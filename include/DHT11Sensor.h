// DHT11Sensor.h
#ifndef DHT11SENSOR_H
#define DHT11SENSOR_H

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Define the pin and sensor type
// Pin Definitions
#define DHTPIN 26  // GPIO2 for the DHT sensor
#define DHTTYPE DHT11

// Create DHT sensor object
extern DHT dht;

// LCD object initialization (update address if required)
extern LiquidCrystal_I2C lcd;

void initializeDHT();
void readDHT11Data();
void displayDHT11Data();

#endif // DHT11SENSOR_H
