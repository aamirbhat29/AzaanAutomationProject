// In DFPlayerManager.cpp
#include "DFPlayerManager.h"
#include "AzaanTimes.h"
#include "TimeManager.h"



HardwareSerial FPSerial(1);  // Define FPSerial using UART1
DFRobotDFPlayerMini myDFPlayer;  // Define myDFPlayer

int currentTrack = 1; // Start with the first track


void playAzaan(int trackNumber) {
  if (trackNumber != currentTrack) {
    currentTrack = trackNumber; // Update the current track
    Serial.print("Playing track number: ");
    Serial.println(currentTrack);
    // Use DFPlayer to play the track
    myDFPlayer.play(currentTrack); 
    simulateButtonPress();
  }
}

void playAzaanDemoTest()
{
   Serial.println("demo starting.....");
    // Use DFPlayer to play the track
    myDFPlayer.play(1); 
    simulateButtonPress();
}

void setupDFPlayer()
{
  FPSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Initializing DFPlayer Mini...");

  int attempts = 0;
  const int maxAttempts = 3;
  bool initialized = false;

  while (attempts < maxAttempts && !initialized) {
    if (myDFPlayer.begin(FPSerial)) {
      initialized = true;
    } else {
      attempts++;
      Serial.print("Attempt ");
      Serial.print(attempts);
      Serial.println(" to initialize DFPlayer Mini failed.");
      delay(1000); // Wait 1 second before retrying
    }
  }

  if (initialized) {
    Serial.println("DFPlayer Mini detected.");
    myDFPlayer.volume(15);             // Set volume (0-30)
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL); // Set EQ mode
  } else {
    Serial.println("Unable to initialize DFPlayer Mini after 3 attempts.");
    // Optionally handle failure (e.g., skip using DFPlayer or reset the board)
  }
}

