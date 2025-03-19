#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <Arduino.h>
#include <Ticker.h>

extern Ticker watchdogTimer;  // Global watchdog timer
extern volatile bool systemFrozen;

void IRAM_ATTR resetESP();
void startWatchdog();
void feedWatchdog();
void stopWatchdog();

#endif
