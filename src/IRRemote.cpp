#include "IRRemote.h"
#include "DFPlayerManager.h"  // Include the DFPlayerManager header
#include "LCDManager.h"
#include <DFRobotDFPlayerMini.h>


// Define and initialize the IR receiver object
IRrecv IrRceiver(IR_RECEIVER_PIN);
decode_results results;
unsigned long lastIRTime = 0;
unsigned long debounceDelay = 200;  // 200 milliseconds debounce time
// Variable to store the current play state
bool isPlaying = false;  // Track whether the player is playing or paused

// Initialize the LCD (assuming address 0x27 and 16x2 screen)
//LiquidCrystal_I2C lcd(0x27, 16, 2); 

// Variables to store previous screen state
String previousLine1 = "";
String previousLine2 = "";
unsigned long volumeDisplayTime = 0;

// Initialize the IR receiver
void initializeIR() {
    IrRceiver.enableIRIn();  // Initialize the IR receiver
    //Serial.begin(115200);
    Serial.println("Initialized IR remote");
    lcd.begin(16, 2);  // Initialize the LCD
    lcd.print("IR Remote Ready");
    delay(2000);
    lcd.clear();
}

// Handle IR remote signals
void handleIRRemote() {
     if (millis() - lastIRTime > debounceDelay) {
    if (IrRceiver.decode(&results)) {
        unsigned long irCode = results.value;  // Assign the IR code directly
        Serial.println(irCode, HEX);  // Print the IR code in HEX format

        // Example cases for recognized IR codes
        switch (irCode) {
            case 0xFFA857: // Volume Up
                Serial.println("Volume Up");
                myDFPlayer.volumeUp();  // Increase volume
               // saveCurrentScreen();  // Save the current screen content
                displayVolume();  // Update the volume on the screen
                volumeDisplayTime = millis();  // Set the time to remove volume display
                break;
            case 0xFFE01F: // Volume Down
                Serial.println("V Down");
                myDFPlayer.volumeDown();  // Decrease volume
                //saveCurrentScreen();  // Save the current screen content
                displayVolume();  // Update the volume on the screen
                volumeDisplayTime = millis();  // Set the time to remove volume display
                break;
            case 0xFFA25D: // CH+
                Serial.println("CH+");
                break;
            case 0xBFFBDE86: // CH
                Serial.println("CH+");
                break;
            case 0xFD82E053: // CH-
                Serial.println("CH+");
                break;
            case 0xFFC23D: // Play/Pause
                Serial.println("Play/Pause");
                if (isPlaying) {
                        myDFPlayer.pause();  // Pause the track
                        isPlaying = false;
                        Serial.println("Paused");
                    } else {
                        myDFPlayer.start();  // Start the track
                        isPlaying = true;
                        Serial.println("Playing");
                    }
                break;
            case 0xFF02FD: // Next
                Serial.println("Next");
                currentTrack++;  // Increment to the next track
                    playAzaan(currentTrack);
                break;
            case 0xFF22DD: // Previous
                Serial.println("Previous");
                if (currentTrack > 1) {
                        currentTrack--;  // Decrement to the previous track
                        playAzaan(currentTrack);
                    }
                break;
            case 0xFF30CF: // 1
                Serial.println("1");
                break;
            case 0xFF18E7: // 2
                Serial.println("2");
                break;
            case 0xFF7A85: // 3
                Serial.println("3");
                break;
            case 0xFF6897: // 0
                Serial.println("0");
                break;
            default:
                Serial.print("Unknown IR Code: ");
                Serial.println(irCode, HEX);
                break;
        }

        IrRceiver.resume();  // Ready to receive the next signal
        lastIRTime = millis();  // Update the last IR time
        }
    }
    // If volume was displayed for 2 seconds, revert to previous screen
    // if (millis() - volumeDisplayTime > 2000) {
    //     revertToPreviousScreen();  // Revert to previous content
    // }
}

// Display current volume on LCD for 2 seconds
//int vol = getVolume();
void displayVolume() {
    int volume = getVolume();  // Get the current volume (0-30)
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Volu ");
    lcd.print(volume);  // Display the current volume level
}

// Revert to the previous screen content
// void revertToPreviousScreen() {
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(previousLine1);  // Re-display previous line 1
//     lcd.setCursor(0, 1);
//     lcd.print(previousLine2);  // Re-display previous line 2
// }

// Function to save the current screen content
// void saveCurrentScreen() {
//     previousLine1 = "";  // Clear previous content
//     previousLine2 = "";
    
//     // Get current content from the LCD
//     for (int i = 0; i < 16; i++) {
//         previousLine1 += char(lcd.read() & 0xFF);
//     }
//     for (int i = 16; i < 32; i++) {
//         previousLine2 += char(lcd.read() & 0xFF);
//     }
// }
