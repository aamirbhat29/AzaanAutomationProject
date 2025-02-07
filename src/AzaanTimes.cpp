#include "AzaanTimes.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LocationManager.h>
#include "DFPlayerManager.h"
#include "TimeManager.h"
#include <Arduino.h> 
#include "LCDManager.h"


// Global variables
String fajrTime = "05:48";
String otherPrayerTimes[] = {"12:45", "22:47", "22:54", "22:59"};
String ipstackKey = "fd070d6f9a4c085dfe619ce5f12a19c1";
String ipstackAPI = "http://api.ipstack.com/check?access_key="+ipstackKey;
String azaanTimeAPIKey = "b91f004f7f391b4f380805620a179d44";
String location = "Delhi%2C%20India";
 bool prayerTimesFetched = false; // Tracks if prayer times have been successfully fetched for the day
 bool prayerTimesUpdateAttempted = false; // To ensure fetchPrayerTimes is called only once
 int fetchRetryCount = 0;                // To count retries
// bool fajrTriggered = false;
// bool dhuhrTriggered = false;
// bool asrTriggered = false;
// bool maghribTriggered = false;
// bool ishaTriggered = false;

void fetchPrayerTimes() {
  Serial.println("Fetching Azaan times from API...");
  // Use HTTPClient to fetch data from the API and parse it.
  delay(1000);
  fetchAzaanTimes();
  // Print updated times
  Serial.println("Azaan times updated:");
  Serial.println("Fajr: " + fajrTime);
  for (int i = 0; i < 4; i++) {
    Serial.println("Prayer " + String(i + 2) + ": " + otherPrayerTimes[i]);
  }

}

void fetchAzaanTimes() {
  if ((WiFi.status() == WL_CONNECTED)) {
    String currentDate;
    String loc;
    unsigned long startTime = millis();  // Record the start time for timeout logic
    const unsigned long timeout = 10000; // 10 seconds timeout for retrying

    // Retry logic with timeout
    while (currentDate == "" && millis() - startTime < timeout) {
        currentDate = getCurrentDate();
        if (currentDate != "") break;  // Break the loop if a valid date is obtained
        delay(500);  // Wait for 500ms before retrying
    }

    if (currentDate == "") {
        Serial.println("Unable to fetch current date after multiple attempts. Aborting calling the API...");
        return;
    }
    String formattedDate = currentDate.substring(8, 10) + "-" + currentDate.substring(5, 7) + "-" + currentDate.substring(0, 4);
    //uncomment after prototype
    // loc = getLocation();
    // loc = formatLocation(loc); 
    String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/"+ formattedDate +"?address="+ location +"&x7xapikey="+azaanTimeAPIKey+"&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6&school=1&midnightMode=0&timezonestring=UTC&calendarMethod=UAQ";
    
    // Debugging logs
    Serial.println("Current Date: " + currentDate);
    Serial.println("LOC: " + loc);
    Serial.println("Constructed API URL: " + curlAPI);


    HTTPClient http;
      http.begin(curlAPI);
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String responseBody = http.getString();
      Serial.println("API Response:");
      Serial.println(responseBody); // Log the raw response for inspection

      // Try parsing prayer times
      bool success = parsePrayerTimes(responseBody);

      // if(success){
      //   prayerTimesFetched = true;
      // }

      if (!success) {
        Serial.println("Failed to parse Azaan times from response.");
      }
    } else {
      Serial.print("HTTP request failed. Code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi is disconnected. Cannot fetch prayer times.");
  }
}

bool parsePrayerTimes(String responseBody) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, responseBody);
  
  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.f_str());
    return false;
  }

    fajrTime = doc["data"]["timings"]["Fajr"].as<String>();
    otherPrayerTimes[0] = doc["data"]["timings"]["Dhuhr"].as<String>();
    otherPrayerTimes[1] = doc["data"]["timings"]["Asr"].as<String>();
    otherPrayerTimes[2] = doc["data"]["timings"]["Maghrib"].as<String>();
    otherPrayerTimes[3] = doc["data"]["timings"]["Isha"].as<String>();

  return true;
}

//simulateButtonPressTo
void simulateButtonPress() {
  pinMode(ADKEY1_PIN, OUTPUT);  // Set ADKEY1_PIN as output
  digitalWrite(ADKEY1_PIN, LOW); // Simulate button press
  delay(200);                   // Wait for 100ms
  digitalWrite(ADKEY1_PIN, HIGH); // Release button
  pinMode(ADKEY1_PIN, INPUT_PULLUP); // Restore ADKEY1_PIN as input
}

int timeToMinutes(String time) {
  int colonIndex = time.indexOf(':');
  int hours = time.substring(0, colonIndex).toInt();
  int minutes = time.substring(colonIndex + 1).toInt();
  return hours * 60 + minutes;
}


String currentPrayerTime[] = {fajrTime, otherPrayerTimes[0], otherPrayerTimes[1], otherPrayerTimes[2], otherPrayerTimes[3]}; // Corresponding prayer times
int lastDisplayedPrayerIndex = -1;  // Tracks the index of the last displayed prayer
bool prayerTimeDisplayed = false; // Flag to track if the prayer time has been displayed


// int getCurrentPrayerIndex() {
//   String now = getCurrentTime();  // Get the current time in HH:MM format

//   // Loop through the prayer times to find the active prayer
//   for (int i = 0; i < 5; i++) {
//     if (now >= currentPrayerTime[i] && (i == 4 || now < currentPrayerTime[i + 1])) {
//       Serial.println('i'+ currentPrayerTime[i]);
//       return i;  // Return the index of the current prayer
//     }
//   }

//   return -1;  // Return -1 if no prayer is active
// }


String currentPrayerName[] = {"FJR", "DHR", "ASR", "MGB", "ISH"};  // List of prayer names
String prayerName = "";
String currentPrayerTimeToDisplay = ""; // Global variable to store the current prayer time
bool prayerDataUpdated = false; 

void checkAndTriggerAzaan() {

   // Display the next prayer time on startup
  // if (!prayerTimesUpdateAttempted) {
  //   displayNextPrayerTime();  // Call function to display the next prayer time based on the current time
  // }
 // Get current time from the TimeManager
  String currentTime = getCurrentTime();

  // Strip seconds from currentTime
  String currentTimeWithoutSeconds = currentTime.substring(0, 5); // Extract "HH:MM"

  // Convert times to minutes for comparison
  int currentMinutes = timeToMinutes(currentTimeWithoutSeconds);
  int fajrMinutes = timeToMinutes(fajrTime);
  int dhuhrMinutes = timeToMinutes(otherPrayerTimes[0]);
  int asrMinutes = timeToMinutes(otherPrayerTimes[1]);
  int maghribMinutes = timeToMinutes(otherPrayerTimes[2]);
  int ishaMinutes = timeToMinutes(otherPrayerTimes[3]);

  // Debugging logs
  Serial.println("Current Time (Minutes): " + String(currentMinutes));
  Serial.println("Fajr Time (Minutes): " + String(fajrMinutes));
  Serial.println("Dhuhr Time (Minutes): " + String(dhuhrMinutes));
  Serial.println("Asr Time (Minutes): " + String(asrMinutes));
  Serial.println("Maghrib Time (Minutes): " + String(maghribMinutes));
  Serial.println("Isha Time (Minutes): " + String(ishaMinutes));


  // Attempt to fetch updated prayer times at 20:45, ensuring it's called only once
  if (currentTimeWithoutSeconds == "02:00") {
         // Reset retry count

    while (!prayerTimesFetched && fetchRetryCount < 3) { // Retry up to 3 times
      fetchPrayerTimes();
      fetchRetryCount++;
      if (prayerTimesFetched) {
        Serial.println("Prayer times successfully fetched and updated.");
        break; // Exit retry loop on success
      } else {
        Serial.println("Retrying to fetch prayer times...");
        delay(5000); // Wait 5 seconds before retrying
      }
    }

    if (!prayerTimesFetched) {
      Serial.println("Failed to fetch prayer times after 3 attempts. Retaining default times.");
    }
    
    prayerTimesUpdateAttempted = true; // Mark the attempt as completed
  } /// end of if block

//     Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
//     Serial.println("fajrTime---"+ fajrTime);
//     for (size_t i = 0; i < sizeof(otherPrayerTimes) / sizeof(otherPrayerTimes[0]); i++) {
//     Serial.print("otherPrayerTimes----: ");
//     Serial.println(otherPrayerTimes[i]);
// }

  // if(currentTimeWithoutSeconds <=fajrTime && currentTimeWithoutSeconds > otherPrayerTimes[3]){
  //   prayerName = currentPrayerName[0];
  //   currentPrayerTimeToDisplay = fajrTime;
  //   Serial.println("prayerTimes: " + fajrTime +"----"+ currentPrayerName[0] +"-----"+ currentPrayerTimeToDisplay);
  //   Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  // }
  // else if(currentTimeWithoutSeconds <=otherPrayerTimes[0] && currentTimeWithoutSeconds > fajrTime){
  //   prayerName = currentPrayerName[1];
  //   currentPrayerTimeToDisplay = otherPrayerTimes[0];
  //   Serial.println("prayerTimes: " + otherPrayerTimes[0] +"----"+ currentPrayerName[1] +"-----"+ currentPrayerTimeToDisplay);
  //   Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  // }
  // else if(currentTimeWithoutSeconds <=otherPrayerTimes[1] && currentTimeWithoutSeconds > otherPrayerTimes[0]){
  //   prayerName = currentPrayerName[2];
  //   currentPrayerTimeToDisplay = otherPrayerTimes[1];
  //   Serial.println("prayerTimes: " + otherPrayerTimes[1] +"----"+ currentPrayerName[2] +"-----"+ currentPrayerTimeToDisplay);
  //   Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  // }
  // else if(currentTimeWithoutSeconds <=otherPrayerTimes[2] && currentTimeWithoutSeconds > otherPrayerTimes[1]){
  //   prayerName = currentPrayerName[3];
  //   currentPrayerTimeToDisplay = otherPrayerTimes[2]; 
  //   Serial.println("prayerTimes: " + otherPrayerTimes[2] +"----"+ currentPrayerName[3] +"-----"+ currentPrayerTimeToDisplay);
  //   Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  // }
  // else if(currentTimeWithoutSeconds <=otherPrayerTimes[3] && currentTimeWithoutSeconds > otherPrayerTimes[2]){
  //   prayerName = currentPrayerName[4];
  //   Serial.println("In check prayerName: "+prayerName);
  //   currentPrayerTimeToDisplay = otherPrayerTimes[3]; 
  //   Serial.println("prayerTimes: " + otherPrayerTimes[3] +"----"+ currentPrayerName[4] +"-----"+ currentPrayerTimeToDisplay);
  //   Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  // }

  // Determine the current prayer time
  if (currentMinutes > ishaMinutes) {
    prayerName = currentPrayerName[0];
    currentPrayerTimeToDisplay = fajrTime;
    Serial.println("prayerTimes: " + fajrTime + "----" + currentPrayerName[0] + "-----" + currentPrayerTimeToDisplay);
    Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  } else if (currentMinutes <= dhuhrMinutes && currentMinutes > fajrMinutes) {
    prayerName = currentPrayerName[1];
    currentPrayerTimeToDisplay = otherPrayerTimes[0];
    Serial.println("prayerTimes: " + otherPrayerTimes[0] + "----" + currentPrayerName[1] + "-----" + currentPrayerTimeToDisplay);
    Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  } else if (currentMinutes <= asrMinutes && currentMinutes > dhuhrMinutes) {
    prayerName = currentPrayerName[2];
    currentPrayerTimeToDisplay = otherPrayerTimes[1];
    Serial.println("prayerTimes: " + otherPrayerTimes[1] + "----" + currentPrayerName[2] + "-----" + currentPrayerTimeToDisplay);
    Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  } else if (currentMinutes <= maghribMinutes && currentMinutes > asrMinutes) {
    prayerName = currentPrayerName[3];
    currentPrayerTimeToDisplay = otherPrayerTimes[2];
    Serial.println("prayerTimes: " + otherPrayerTimes[2] + "----" + currentPrayerName[3] + "-----" + currentPrayerTimeToDisplay);
    Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  } else if (currentMinutes <= ishaMinutes && currentMinutes > maghribMinutes) {
    prayerName = currentPrayerName[4];
    currentPrayerTimeToDisplay = otherPrayerTimes[3];
    Serial.println("prayerTimes: " + otherPrayerTimes[3] + "----" + currentPrayerName[4] + "-----" + currentPrayerTimeToDisplay);
    Serial.println("currentTimeWithoutSeconds: " + currentTimeWithoutSeconds);
  }

  // After updating prayerName and currentPrayerTimeToDisplay
  prayerDataUpdated = true; // Set the flag to indicate data is updated

  // Check if it's time for a prayer
  // if (currentTimeWithoutSeconds == fajrTime) {
  //   Serial.println("Time for Fajr Azaan!");
  //   playAzaan(1); // Play Azaan for Fajr
  //   //displayPT(fajrTime); // Display Dhuhr time after Fajr Azaan
  // } 
  // else if (currentTimeWithoutSeconds == otherPrayerTimes[0]) {
  //   Serial.println("Time for Dhuhr Azaan!" + otherPrayerTimes[0]);
  //   playAzaan(2); // Play Azaan for Dhuhr
  //   //displayPT(otherPrayerTimes[0]); // Display Asr time after Dhuhr Azaan
  // } 
  // else if (currentTimeWithoutSeconds == otherPrayerTimes[1]) {
  //   Serial.println("Time for Asr Azaan!" + otherPrayerTimes[1]);
  //   playAzaan(2); // Play Azaan for Asr
  //   //displayPT(otherPrayerTimes[1]); // Display Asr time after Dhuhr Azaan
  // } 
  // else if (currentTimeWithoutSeconds == otherPrayerTimes[2]) {
  //   Serial.println("Time for Maghrib Azaan!");
  //   playAzaan(2); // Play Azaan for Maghrib
  //   //displayPT(otherPrayerTimes[2]); // Display Asr time after Dhuhr Azaan
  // } 
  // else if (currentTimeWithoutSeconds == otherPrayerTimes[3]) {
  //   Serial.println("Time for Isha Azaan!");
  //   playAzaan(2); // Play Azaan for Isha
  //   //displayPT(otherPrayerTimes[3]); // Display Asr time after Dhuhr Azaan
  // }


  // Check if it's time for a prayer
  if (currentMinutes == fajrMinutes) {
    Serial.println("Time for Fajr Azaan!");
    playAzaan(1); // Play Azaan for Fajr
  } else if (currentMinutes == dhuhrMinutes) {
    Serial.println("Time for Dhuhr Azaan!");
    playAzaan(2); // Play Azaan for Dhuhr
  } else if (currentMinutes == asrMinutes) {
    Serial.println("Time for Asr Azaan!");
    playAzaan(2); // Play Azaan for Asr
  } else if (currentMinutes == maghribMinutes) {
    Serial.println("Time for Maghrib Azaan!");
    playAzaan(2); // Play Azaan for Maghrib
  } else if (currentMinutes == ishaMinutes) {
    Serial.println("Time for Isha Azaan!");
    playAzaan(2); // Play Azaan for Isha
  }

  // Serial.println("Updated prayerName: " + prayerName);
  // Serial.println("Updated currentPrayerTimeToDisplay: " + currentPrayerTimeToDisplay);
}
