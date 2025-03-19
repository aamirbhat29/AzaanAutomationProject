#include "watchDog.h"

Ticker watchdogTimer;
volatile bool systemFrozen = false;

void IRAM_ATTR resetESP() { 
    if (systemFrozen) {  // Only reset if the system is unresponsive
        Serial.println("🚨 Watchdog detected a freeze! Restarting ESP...");
        ESP.restart();
    }
}

void startWatchdog() {
    systemFrozen = false;  // System is working
    watchdogTimer.attach(5, resetESP);  // Check every 5 seconds
    Serial.println("🕒 Watchdog started!");
}

void feedWatchdog() {
    systemFrozen = false;  // Reset the freeze flag
    Serial.println("✅ Watchdog fed, system is alive.");
}

void stopWatchdog() {
    watchdogTimer.detach();
    Serial.println("🛑 Watchdog stopped!");
}
