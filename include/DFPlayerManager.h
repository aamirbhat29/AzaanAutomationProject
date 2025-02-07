// In DFPlayerManager.h
#ifndef DFPlayerManager_h
#define DFPlayerManager_h

#include <Arduino.h>
#include <DFRobotDFPlayerMini.h>
#include <HardwareSerial.h>

#define RX_PIN 16  // RX pin connected to DFPlayer TX
#define TX_PIN 17  // TX pin connected to DFPlayer RX
#define ADKEY1_PIN 4 // GPIO pin connected to ADKEY1 of DFPlayer Mini
    void playAzaanDemoTest();
    void setupDFPlayer();
    void playAzaan(int trackNumber);
    int getVolume();
    extern int currentTrack;
    extern HardwareSerial FPSerial;  // Declare FPSerial
    extern DFRobotDFPlayerMini myDFPlayer;  // Declare myDFPlayer


#endif
