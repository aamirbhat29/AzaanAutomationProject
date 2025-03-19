// In DFPlayerManager.cpp
#include "DFPlayerManager.h"
#include "AzaanTimes.h"
#include "TimeManager.h"
#include "watchDog.h"
#include <esp_task_wdt.h>  // Include watchdog library
#include <EEPROM.h>  // ✅ Include EEPROM library

#define WDT_TIMEOUT 5  // Set watchdog timeout to 3 seconds


 HardwareSerial FPSerial(1);  // Define FPSerial using UART1
 SoftwareSerial mySerial(RX_PIN, TX_PIN);
 DFRobotDFPlayerMini myDFPlayer;

void printDetail(uint8_t type, int value);

int currentTrack = 0; // Start with the first track
int currentVolume = 20; // Default volume

void playTrackX(byte x) {
    mySerial.write((byte)0x7E);
    mySerial.write((byte)0xFF);
    mySerial.write((byte)0x06);
    mySerial.write((byte)0x03);
    mySerial.write((byte)0x00);
    mySerial.write((byte)0x00);
    mySerial.write((byte)x);
    mySerial.write((byte)0xEF);
}

void printDetail(uint8_t type, int value) {
    delay(500);
    switch (type) {
        case TimeOut:
            Serial.println(F("Time Out!"));
            break;
        case WrongStack:
            Serial.println(F("Stack Wrong!"));
            break;
        case DFPlayerCardInserted:
            Serial.println(F("Card Inserted!"));
            break;
        case DFPlayerCardRemoved:
            Serial.println(F("Card Removed!"));
            break;
        case DFPlayerCardOnline:
            Serial.println(F("Card Online!"));
            break;
        case DFPlayerUSBInserted:
            Serial.println("USB Inserted!");
            break;
        case DFPlayerUSBRemoved:
            Serial.println("USB Removed!");
            break;
        case DFPlayerPlayFinished:
            Serial.print(F("Number:"));
            Serial.print(value);
            Serial.println(F(" Play Finished!"));
            break;
        case DFPlayerError:
            Serial.print(F("DFPlayerError:"));
            switch (value) {
                case Busy:
                    Serial.println(F("Card not found"));
                    break;
                case Sleeping:
                    Serial.println(F("Sleeping"));
                    break;
                case SerialWrongStack:
                    Serial.println(F("Get Wrong Stack"));
                    break;
                case CheckSumNotMatch:
                    Serial.println(F("Check Sum Not Match"));
                    break;
                case FileIndexOut:
                    Serial.println(F("File Index Out of Bound"));
                    break;
                case FileMismatch:
                    Serial.println(F("Cannot Find File"));
                    break;
                case Advertise:
                    Serial.println(F("In Advertise"));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void playAzaan(int trackNumber) {
  Serial.print("Playing track number: ");
  Serial.println(trackNumber);
  Serial.print("Playing currentTrack: ");
  Serial.println(currentTrack);
  if (trackNumber != currentTrack) {


   // myDFPlayer.stop();
    delay(500);
    
    currentTrack = trackNumber;

    myDFPlayer.playMp3Folder(currentTrack);
        printDetail(myDFPlayer.readType(), myDFPlayer.read());
    myDFPlayer.volume(30);
    
      delay(1000); // Longer delay for track buffering
      
  }
}

void checkDFPlayerStatus() {
    Serial.println("🔍 Checking DFPlayer status...");
    if (!myDFPlayer.available()) {
        Serial.println("❌ DFPlayer Mini is NOT responding!");
    } else {
        Serial.println("✅ DFPlayer Mini is responding.");
    }
}

void playAzaanDemoTest()
{
   Serial.println("demo starting.....");
    // Use DFPlayer to play the track
    myDFPlayer.play(2); 
    simulateButtonPress();
}

int getVolume() {
    int volume = myDFPlayer.readVolume();  // Get the current volume (0-30)
    return volume;
}

int getVolumeManually() {
    return currentVolume;  // Return stored volume level
}

void increaseVolume() {
    if (currentVolume < 30) {
        currentVolume++;
        myDFPlayer.volume(currentVolume);
        Serial.print("Volume Increased: ");
        Serial.println(currentVolume);
    } else {
        Serial.println("Volume is already at maximum (30).");
    }
}

void decreaseVolume() {
    if (currentVolume > 0) {
        currentVolume--;
        myDFPlayer.volume(currentVolume);
        Serial.print("Volume Decreased: ");
        Serial.println(currentVolume);
    } else {
        Serial.println("Volume is already at minimum (0).");
    }
}

void setupDFPlayer()
{
  #if defined(ESP32)
    FPSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
    #else
    FPSerial.begin(9600);
    #endif

    Serial.begin(115200);
    Serial.println(F("DFRobot DFPlayer Mini Demo"));
    Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
    
    if (!myDFPlayer.begin(FPSerial, true, true)) {
        Serial.println(F("Unable to begin:"));
        Serial.println(F("1. Please recheck the connection!"));
        Serial.println(F("2. Please insert the SD card!"));
        while (true) {
            delay(0);
        }
    }
    Serial.println(F("DFPlayer Mini online."));
    myDFPlayer.volume(10);
}

void resetDFPlayer() {
    Serial.println(F("🔄 Resetting DFPlayer..."));
    FPSerial.end();  // Stop serial communication
    delay(500);
    FPSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN); // Restart serial
    delay(500);
    myDFPlayer.begin(FPSerial, true, true); // Reinitialize DFPlayer
    myDFPlayer.volume(10);
    delay(500);
}


