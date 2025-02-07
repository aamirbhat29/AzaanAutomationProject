// IRRemote.h
#ifndef IRREMOTE_H
#define IRREMOTE_H

#include <IRremoteESP8266.h>
#include <IRrecv.h>


// Define the pin for the IR receiver
#define IR_RECEIVER_PIN 15

// Declare the IR receiver object as external
extern IRrecv IrRceiver;
// Initialize the IR receiver object
void initializeIR();
void handleIRRemote();
void displayVolume();

#endif // IRREMOTE_H
