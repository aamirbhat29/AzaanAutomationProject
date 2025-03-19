// =============================================
// Configuration Constants
#include "AzaanTimes.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <LocationManager.h>
#include "DFPlayerManager.h"
#include "TimeManager.h"
#include <Arduino.h> 
#include "LCDManager.h"


// Global variables
String sunRiseTime;
String fajrTime = "18:46";
String otherPrayerTimes[] = {"23:56", "23:59", "22:55", "23:25"};
String ipstackKey = "fd070d6f9a4c085dfe619ce5f12a19c1";
String ipstackAPI = "http://api.ipstack.com/check?access_key="+ipstackKey;
String azaanTimeAPIKey = "b91f004f7f391b4f380805620a179d44";
//String location = "Delhi%2C%20India";
 bool prayerTimesFetched = false; // Tracks if prayer times have been successfully fetched for the day
 bool prayerTimesUpdateAttempted = false; // To ensure fetchPrayerTimes is called only once
 int fetchRetryCount = 0;                // To count retries
 // Add these at the top of your code (global scope)
bool maghribSequenceActive = false;
unsigned long duaStartTime = 0;
bool duaPlayed = false;

// Fetch location data using IP
String countryName;  // Global variable
String regionName;   // Global variable
String city;         // Global variable

// String getLocation() {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin(ipstackAPI);  
//     int httpCode = http.GET();

//     if (httpCode == 200) {
//       String payload = http.getString();
//       StaticJsonDocument<1024> doc;
//       DeserializationError error = deserializeJson(doc, payload);

//       if (!error) {

//         // Check if API response has "success": false
//         if (doc.containsKey("success") && doc["success"] == false) {
//           Serial.print("API Error: ");
//           Serial.println(doc["error"]["info"].as<String>());
//           return "Srinagar, India";  // Return default location
//         }

//         // Extract and combine values into a location string
//         countryName = doc["country_name"].as<String>();
//         regionName = doc["region_name"].as<String>();
//         city = doc["city"].as<String>();
//         String location = city + ", " + regionName + ", " + countryName;

//         // Print the stored values
//         //Serial.println(payload);
//         Serial.println("Country: " + countryName);
//         Serial.println("Region: " + regionName);
//         Serial.println("City: " + city);
//         Serial.println("Location: " + location);

//         return location; // Return the location string
//         // location.replace(", ", "%2C%20");
//         // return location;
//       } else {
//         Serial.print("JSON Parse Error: ");
//         Serial.println(error.c_str());
//         return "Srinagar, India";  // Return default location on parsing failure
//       }
//     } else {
//       Serial.println("HTTP Request Failed");
//       return "Srinagar, India";  // Return default location on HTTP failure
//     }
//     http.end();
//   }
//   return "Srinagar, India"; // Default return in case of failure
// }

String googleApiKey = "AIzaSyC-cqrcNOuA0DLJhZUFS-SjjoDeM06QpSo";  // Replace with your API key
String googleGeolocationAPI = "https://www.googleapis.com/geolocation/v1/geolocate?key=" + googleApiKey;

// Default location (Noida, Uttar Pradesh, India)
String defaultCity = "Srinagar";
String defaultState = "Jammu & Kashmir";
String defaultCountry = "India";

struct Location {
  String city;
  String state;
  String country;
};

// Function to get City, State, Country from Latitude & Longitude
Location getCityStateCountry(float lat, float lng) {
  String reverseGeocodeURL = "https://maps.googleapis.com/maps/api/geocode/json?latlng=" + 
                              String(lat, 6) + "," + String(lng, 6) + "&key=" + googleApiKey;
  
  HTTPClient http;
  http.begin(reverseGeocodeURL);
  int httpCode = http.GET();
  String city, state, country;

  if (httpCode == 200) {
    String payload = http.getString();
    http.end();

    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, payload);
    
    if (!error) {
      JsonArray results = doc["results"].as<JsonArray>();

      for (JsonObject result : results) {
        JsonArray address_components = result["address_components"].as<JsonArray>();

        for (JsonObject component : address_components) {
          JsonArray types = component["types"].as<JsonArray>();
          String type = types[0].as<String>();

          if (type == "locality") {
            city = component["long_name"].as<String>();
          } else if (type == "administrative_area_level_1") {
            state = component["long_name"].as<String>();
          } else if (type == "country") {
            country = component["long_name"].as<String>();
          }
        }
        if (city.length() > 0 && state.length() > 0 && country.length() > 0) {
          break;
        }
      }
      Serial.println("Detected Location: " + city + ", " + state + ", " + country);
      return {city, state, country};
    }
  }
  http.end();
  Serial.println("Returning Default Location due to API failure.");
  return {defaultCity, defaultState, defaultCountry};  // Default if API fails
}

// Function to get city, state, and country
Location getLocation() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(googleGeolocationAPI);
    http.addHeader("Content-Type", "application/json");

    // Scan available Wi-Fi networks
    int numNetworks = WiFi.scanNetworks();
    String jsonPayload = "{\"wifiAccessPoints\":[";

    for (int i = 0; i < numNetworks; i++) {
      if (i > 0) jsonPayload += ",";
      jsonPayload += "{\"macAddress\":\"" + WiFi.BSSIDstr(i) + "\",\"signalStrength\":" + String(WiFi.RSSI(i)) + "}";
    }
    jsonPayload += "]}";

    // Send POST request
    int httpCode = http.POST(jsonPayload);
    String payload = http.getString();
    http.end();

    if (httpCode == 200) {
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, payload);

      if (!error) {
        float lat = doc["location"]["lat"];
        float lng = doc["location"]["lng"];

        Serial.printf("Latitude: %.6f, Longitude: %.6f\n", lat, lng);

        // Get City, State, Country using Reverse Geocoding
        return getCityStateCountry(lat, lng);
      } else {
        Serial.println("JSON Parsing Error!");
      }
    } else {
      Serial.println("HTTP Request Failed");
    }
  }
  Serial.println("Returning Default Location: " + defaultCity + ", " + defaultState + ", " + defaultCountry);
  return {defaultCity, defaultState, defaultCountry};  // Default location if Wi-Fi or API fails
}


String formatLocation(String newLocation) {
    newLocation.replace(", ", "%2C%20"); // Replace ", " with "%2C%20"
    newLocation.replace(",", "%2C%20");  // Replace any remaining "," with "%2C%20"
    newLocation.replace(" ", "");        // Remove all spaces
    return newLocation;
}


void fetchPrayerTimes() {
  Serial.println("Fetching Azaan times from API...");
  // Use HTTPClient to fetch data from the API and parse it.
  delay(1000);
  fetchAzaanTimes();
  // Print updated times
  Serial.println("Azaan times updated:");
  Serial.println("Fajr: " + fajrTime);
  Serial.println("Sunrise: "+sunRiseTime);
  for (int i = 0; i < 4; i++) {
    Serial.println("Prayer " + String(i + 2) + ": " + otherPrayerTimes[i]);
  }

}
void showLocationBriefly(String location) {
    lcd.clear();

    // Extract first 16 and next 16 characters for two lines
    String line1 = location.substring(0, 16);
    String line2 = location.substring(16, 32);

    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);

    // Set flag and timestamp to track when to reset display
    showingLocation = true;
    locationStartTime = millis();
}

void fetchAzaanTimes() {
  if ((WiFi.status() == WL_CONNECTED)) {
    String currentDate;
    Location loc;
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
     loc = getLocation();
     String newLocation = loc.city+loc.state+loc.country;
     Serial.println("newLocationWithoutFormat: " + newLocation);
     newLocation = formatLocation(newLocation);
     showLocationBriefly(newLocation);

    //String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/"+ formattedDate +"?address="+ loc +"&x7xapikey="+azaanTimeAPIKey+"&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6&school=1&midnightMode=0&timezonestring=Asia/Kolkata&calendarMethod=UAQ";
    String curlAPI = "https://api.aladhan.com/v1/timingsByAddress/"+ formattedDate +"?address="+ newLocation +"&x7xapikey="+azaanTimeAPIKey+"&method=3&shafaq=general&tune=4%2C0%2C0%2C0%2C0%2C0%2C0%2C4%2C-6&school=1&midnightMode=0&timezonestring=Asia/Kolkata&calendarMethod=UAQ";
    // Debugging logs
    Serial.println("Current Date: " + currentDate);
    Serial.println("newLocationFormatted: " + newLocation);
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
    sunRiseTime = doc["data"]["timings"]["Sunrise"].as<String>();
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
  delay(100);                   // Wait for 100ms
  digitalWrite(ADKEY1_PIN, HIGH); // Release button
  pinMode(ADKEY1_PIN, INPUT_PULLUP); // Restore ADKEY1_PIN as input
  delay(100);
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


String currentPrayerName[] = {"FAJR", "DHUR", "ASAR", "MGRB", "ISHA"};  // List of prayer names
String sunRiseTimeToShowLabel = "SR";
String prayerName = "";
String currentPrayerTimeToDisplay = ""; // Global variable to store the current prayer time
bool prayerDataUpdated = false; 

void checkAndTriggerAzaan() {

 // Get current time from the TimeManager
  String currentTime = getCurrentTime();

  // Strip seconds from currentTime
  String currentTimeWithoutSeconds = currentTime.substring(0, 5); // Extract "HH:MM"

  int updateTimeMinutes = timeToMinutes("02:00"); // Convert "02:00" to minutes
  // Convert times to minutes for comparison
  int currentMinutes = timeToMinutes(currentTimeWithoutSeconds);
  int fajrMinutes = timeToMinutes(fajrTime);
  // Serial.println("fajrMinutes: ");
  // Serial.print(fajrMinutes);
  int sehriTimeMinutes = fajrMinutes - 35;
  // Serial.println("sehriMinutes: ");
  // Serial.print(sehriTimeMinutes);
  int sunRiseMinutes = timeToMinutes(sunRiseTime);
  int dhuhrMinutes = timeToMinutes(otherPrayerTimes[0]);
  int asrMinutes = timeToMinutes(otherPrayerTimes[1]);
  int iftarTimeInMinutes = timeToMinutes(otherPrayerTimes[2]);
  int maghribMinutes = iftarTimeInMinutes + 1; // Simply add 1 minute
  int ishaMinutes = timeToMinutes(otherPrayerTimes[3]);

  // Attempt to fetch updated prayer times at 20:45, ensuring it's called only once
// Check if it's exactly 2:00 AM and ensure it's fetched only once
if (currentMinutes == updateTimeMinutes && !prayerTimesUpdateAttempted) {
    fetchRetryCount = 0; // Reset retry count

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

    prayerTimesUpdateAttempted = true; // Mark update as done for the day
}

  // if (currentTimeWithoutSeconds == "02:00") {
  //        // Reset retry count

  //   while (!prayerTimesFetched && fetchRetryCount < 3) { // Retry up to 3 times
  //     fetchPrayerTimes();
  //     fetchRetryCount++;
  //     if (prayerTimesFetched) {
  //       Serial.println("Prayer times successfully fetched and updated.");
  //       break; // Exit retry loop on success
  //     } else {
  //       Serial.println("Retrying to fetch prayer times...");
  //       delay(5000); // Wait 5 seconds before retrying
  //     }
  //   }

  //   if (!prayerTimesFetched) {
  //     Serial.println("Failed to fetch prayer times after 3 attempts. Retaining default times.");
  //   }
    
  //   prayerTimesUpdateAttempted = true; // Mark the attempt as completed
  // } /// end of if block

  // Determine the current prayer time
  if ((currentMinutes > ishaMinutes && currentMinutes <= 1440) || (currentMinutes >= 0 && currentMinutes <= fajrMinutes)) {
    prayerName = currentPrayerName[0];
    currentPrayerTimeToDisplay = fajrTime;
  }  else if (currentMinutes > fajrMinutes && currentMinutes < sunRiseMinutes) {
    prayerName = sunRiseTimeToShowLabel;
    currentPrayerTimeToDisplay = sunRiseTime;
  }
  else if (currentMinutes <= dhuhrMinutes && currentMinutes >= sunRiseMinutes) {
    prayerName = currentPrayerName[1];
    currentPrayerTimeToDisplay = otherPrayerTimes[0];
  } else if (currentMinutes <= asrMinutes && currentMinutes > dhuhrMinutes) {
    prayerName = currentPrayerName[2];
    currentPrayerTimeToDisplay = otherPrayerTimes[1];
  } else if (currentMinutes <= maghribMinutes && currentMinutes > asrMinutes) {
    prayerName = currentPrayerName[3];
    currentPrayerTimeToDisplay = otherPrayerTimes[2];
  } else if (currentMinutes <= ishaMinutes && currentMinutes > maghribMinutes) {
    prayerName = currentPrayerName[4];
    currentPrayerTimeToDisplay = otherPrayerTimes[3];
  }

  // After updating prayerName and currentPrayerTimeToDisplay
  prayerDataUpdated = true; // Set the flag to indicate data is update


  // Check if it's time for a prayer
  if(currentMinutes == sehriTimeMinutes ) {
    Serial.println("Time to ear Sehri!");
    playAzaan(7); // Play Azaan for Fajr
  } 
  else if (currentMinutes == fajrMinutes) {
    Serial.println("Time for Fajr Azaan!");
    playAzaan(1); // Play Azaan for Fajr
  }
  else if (currentMinutes == dhuhrMinutes) {
    Serial.println("Time for Dhuhr Azaan!");
    playAzaan(2); // Play Azaan for Dhuhr
  } else if (currentMinutes == asrMinutes) {
    Serial.println("Time for Asr Azaan!");
    playAzaan(3); // Play Azaan for Asr
  }
  else if (currentMinutes == iftarTimeInMinutes) {
    Serial.println("Time for Iftaar!");
    playAzaan(4); 
} 
  else if (currentMinutes == maghribMinutes) {
    Serial.println("Time for Magrib!");
    playAzaan(5); // Play Azaan for Magrib
  }
  else if (currentMinutes == ishaMinutes) {
    Serial.println("Time for Isha Azaan!");
    playAzaan(6); // Play Azaan for Isha
  }
}
