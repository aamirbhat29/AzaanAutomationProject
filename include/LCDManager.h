// LCDManager.h
#ifndef LCDMANAGER_H
#define LCDMANAGER_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>

extern LiquidCrystal_I2C lcd;

void initializeLCD();
void displayMessage(String message);
void displayTime(String currentTime);
void updateDHTValues(float temp, float hum); // New function for DHT integration

#endif // LCDMANAGER_H