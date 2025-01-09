#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_PASSWORD";
String api = "YOUR_WRITE_API_KEY"; // Write Thingspeak Channel API Key
String sec_api = "OTHER_WRITE_API_KEY"; // Write Thingspeak Channel API Key for the other app

// LED pin setup
int greenLED = 14;  // Pin for green LED
int yellowLED = 12; // Pin for orange LED
int redLED = 13;   // Pin for red LED

void setup() {
  Serial.begin(115200);
  // Set up LED pins
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  // Set field8 to 0 for both channels
  setField8("0", sec_api);
  setField8("0", api);
}

void sendData(String green, String orange, String red) {
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field1=" + green + "&field2=" + orange + "&field3=" + red;
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to WiFi
    HTTPClient http;
    http.begin(url); // Specify the URL

    int httpResponseCode = http.GET(); // Send the GET request

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code (sendData): ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error on sending request: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void setField8(String value, String api) {
  // Construct and send the HTTP GET request
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field8=" + value;
   if (WiFi.status() == WL_CONNECTED) { // Check if connected to WiFi
    HTTPClient http;
    http.begin(url); // Specify the URL

    int httpResponseCode = http.GET(); // Send the GET request

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code (setField8): ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error on sending request: ");
      Serial.println(httpResponseCode);
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(2000);
}

void controlTrafficLight(String state) {
  if (state == "green") {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (state == "orange") {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else if (state == "red") {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

int getField8(String api) {
  String url = "http://api.thingspeak.com/channels/2704086/fields/8.json?api_key=" + api + "&results=1";
  HTTPClient http;
  http.begin(url); // Specify the URL

  int httpResponseCode = http.GET(); // Send the GET request

  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.println("JSON Response: ");
    Serial.println(payload);  // Print the full response

    // Call helper functions to parse the JSON and extract 'field8' value
    String feedsSection = extractFeedsSection(payload);
    int field8Value = extractField8Value(feedsSection);
    
    return field8Value;
  } else {
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
    return -1;  // Return -1 if request fails
  }

  http.end(); // Free resources
}

// Function to extract the 'feeds' section from the JSON response
String extractFeedsSection(String payload) {
  int startIdx = payload.indexOf("\"feeds\":[");
  int endIdx = payload.indexOf("]}", startIdx); // Closing braces for the feeds array
  if (startIdx == -1 || endIdx == -1) {
    Serial.println("Error: 'feeds' section not found.");
    return "";  // Return empty if 'feeds' section is not found
  }
  return payload.substring(startIdx + 8, endIdx + 1); // Extract the feeds section
}

// Function to extract the 'field8' value from the 'feeds' section
int extractField8Value(String feedsSection) {
  int field8Pos = feedsSection.indexOf("\"field8\":\"");
  if (field8Pos == -1) {
    Serial.println("Error: 'field8' value not found.");
    return -1; // Return -1 if 'field8' is not found
  }
  String field8String = feedsSection.substring(field8Pos + 10, field8Pos + 11); // Extract "1" or "0"
  return field8String.toInt();  // Convert to integer and return
}

long lastField8Update = 0;
long lastCycleUpdate = 0;
const long field8Interval = 1 * 60 * 1000;  // 1 minute (60 seconds)
const long field8Duration = 20 * 1000;      // 20 seconds
bool field8Active = false;

void loop() {
  unsigned long currentTime = millis();  // Get the current time in milliseconds

  if (currentTime - lastField8Update >= field8Interval && !field8Active) {
    setField8("1", api);  // Activate Field 8
    field8Active = true;
    lastField8Update = currentTime;
    Serial.println("Field 8 activated");
  }

  // Check if Field 8 has been active for 1 minute, then deactivate it
  if (field8Active && currentTime - lastField8Update >= field8Duration) {
    setField8("0", api);  // Deactivate Field 8
    field8Active = false;
    lastField8Update = currentTime;
    Serial.println("Field 8 deactivated");
  }

  // Get the current value of Field 8
  int field8Value = getField8(sec_api);  
  Serial.print("Current Field 8 value: ");
  Serial.println(field8Value);

  // Override normal traffic light behavior if Field 8 is active
  if (field8Value == 1) {
    controlTrafficLight("orange");
    sendData("0", "1", "0");  // Send data for orange light
  } else {
    // Normal traffic light behavior
    if (currentTime - lastCycleUpdate >= 20000) {  // 20 seconds per light
      controlTrafficLight("red");
      sendData("0", "0", "1");  // Send data for red light
      delay(30000);

      controlTrafficLight("green");
      sendData("1", "0", "0");  // Send data for green light
      delay(30000);

      controlTrafficLight("orange");
      sendData("0", "1", "0");  // Send data for orange light
      delay(20000);

      lastCycleUpdate = currentTime;  // Update the last cycle time
    }
  }
}