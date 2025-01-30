#include "LEDManager.h"
#include <WiFi.h>

// Define the pin numbers for the LEDs (GPIO pins for ESP32)
const int wifiPin = 14;         // GPIO14 for Wi-Fi LED
const int dfPlayerPin = 5;     // GPIO5 for DFPlayer LED
const int dhtPin = 2;          // GPIO2 for DHT LED (DHT11)
const int rtcPin = 12;         // GPIO12 for RTC LED
const int esp32Pin = 13;       // GPIO13 for ESP32 LED

// Variables for LED blinking
unsigned long previousMillis = 0;  // Timer for blinking
bool ledState = LOW;               // LED state

extern bool wifiConnecting; // Reference to the flag in WiFiManager

// Initialize the LED pins (called in setup())
void beginLEDManager() {
    pinMode(wifiPin, OUTPUT);
    digitalWrite(wifiPin, LOW);  // Initialize LED as off
    pinMode(dfPlayerPin, OUTPUT);
    pinMode(dhtPin, OUTPUT);
    pinMode(rtcPin, OUTPUT);
    pinMode(esp32Pin, OUTPUT);
}

// Implementation of setWiFiStatus()
void setWiFiStatus(bool connected) {
  if (connected) {
    digitalWrite(wifiPin, HIGH);  // Solid light for connected status
  } else {
    digitalWrite(wifiPin, LOW);  // Turn off the LED for disconnected status
  }
}

// Helper function for blinking LED
void blinkLED(int pin, int interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;  // Toggle LED state
    digitalWrite(pin, ledState);
  }
}

// Call this in loop() to handle Wifi LED State
void handleWifiLEDState() {
  if (WiFi.status() != WL_CONNECTED) {
    // If disconnected, blink LED to indicate connection attempt
    blinkLED(wifiPin, 500);
  }
}

