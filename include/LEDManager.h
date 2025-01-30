#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include <Arduino.h>

// Pin assignments for each LED
extern const int wifiPin;
extern const int dfPlayerPin;
extern const int dhtPin;
extern const int rtcPin;
extern const int esp32Pin;

// Initialize the LED pins
void beginLEDManager();

// Control LED behavior for Wi-Fi status
void setWiFiStatus(bool connected);

// for exposing the Wifi Led functionality
void handleWifiLEDState();

// Control LED behavior for DFPlayer status
void setDFPlayerStatus(bool playing);

// Control LED behavior for DHT sensor status
void setDHTStatus(bool healthy);

// Control LED behavior for RTC status
void setRTCStatus(bool running);

// Control LED behavior for ESP32 status
void setESP32Status(bool running);

// Control blinking LED for connecting status
void blinkLED(int pin, int interval);

#endif
