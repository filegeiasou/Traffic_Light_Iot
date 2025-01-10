#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
String api = "YOUR_WRITE_API_KEY";          // Write Thingspeak Channel API Key
String read_api_key = "YOUR_READ_API_KEY";  // Read API key for ThingSpeak
String channel_id = "YOUR_CHANNEL_ID";      // Channel ID for ThingSpeak

// LED Pins
int greenLED = 14;
int yellowLED = 12;
int redLED = 13;

// Traffic light durations
unsigned long redTime = 30000;       // Red light duration time
unsigned long greenTime = 30000;     // Green light duration time
unsigned long orangeTime = 20000;    // Orange light duration time

// Variables
unsigned long previousMillis = 0;        // Tracks last light change
unsigned long alertStartTime = 0;        // Tracks start of alert mode
unsigned long checkInterval = 5000;      // Check ThingSpeak Read Channel every 5 seconds
unsigned long previousUpdateMillis = 0;
unsigned long previousFieldCheckMillis = 0;
unsigned long updateInterval = 600000;    // Update interval (every 10 minutes)
unsigned long alertDuration = 60000;
unsigned long previousAlertMillis = 0;  // To track time for alert mode (1 minute)
int currentLight = 0;                    // 0 = Red, 1 = Green, 2 = Orange
bool alertMode = false;                  // Alert mode state status

void setup() {
  Serial.begin(115200);

  // Setup LED pins
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  setField8(0); // Initially set field8 value to 0
}

void controlTrafficLight(String state) {
  if (state == "red") {
    Serial.println("Red Light ON");
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
  } else if (state == "green") {
    Serial.println("Green Light ON");
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
  } else if (state == "orange") {
    Serial.println("Orange Light ON");
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
  }
}

int getField8() {
  String url = "http://api.thingspeak.com/channels/" + channel_id + "/fields/8/last?api_key=" + read_api_key;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      String payload = http.getString();
      // Serial.print("Field 8 Value: ");
      // Serial.println(payload);
      http.end();
      delay(100); // Delay to stabilize communication
      return payload.toInt();
    } else {
      Serial.println("Error reading ThingSpeak Channel");
      http.end();
      delay(100); // Delay to handle failed read
      return -1;
    }
  }
  return -1;
}

void sendData(String green, String orange, String red) {
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field1=" + green + "&field2=" + orange + "&field3=" + red;
  if (WiFi.status() == WL_CONNECTED) { // Check if connected to WiFi
    HTTPClient http;
    http.begin(url); // Specify the URL

    int httpResponseCode = http.GET(); // Send the GET request

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
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

void setField8(int value) {
  unsigned long currentMillis = millis();

  // Throttle updates to avoid flooding ThingSpeak
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field8=" + String(value);
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      // Serial.print("Updated Field 8 to: ");
      // Serial.println(value);
      delay(500); // Allow time for the ThingSpeak channel to process the update
    } else {
      Serial.println("Error updating Field 8");
      delay(500); // Handle error gracefully with a delay
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // // Update field8 every 10 minutes
  // if (currentMillis - previousUpdateMillis >= updateInterval) {
  //   previousUpdateMillis = currentMillis;

  //   // Set field8 to 1 or 0 (depending on your logic)
  //   setField8(1); // Or setField8(0) depending on the condition

  //   Serial.println("Field 8 has been updated");
  // }

  // Check for keystroke to enter alert mode
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'e' && !alertMode) {
      alertStartTime = currentMillis; // Start alert mode
      setField8(1); // Update field8 to 1
      // delay(2000);
      Serial.println("Entered Alert Mode via Keystroke!");
    }
  }

  // Check field8 value at the specified interval (2 seconds)
  if (currentMillis - previousFieldCheckMillis >= checkInterval) {
    previousFieldCheckMillis = currentMillis;  // Update the last field check time

    int field8Value = getField8();
    Serial.print("Field 8 Value: ");
    Serial.println(field8Value);

    // If the field8 value is 1, enter alert mode
    if (field8Value == 1) {
      // alertMode = true;
      previousAlertMillis = currentMillis;
      Serial.println("Entered alert mode");
      controlTrafficLight("orange");
      delay(15000);
      sendData("0", "1", "0");
      delay(alertDuration);
      Serial.println("Alert mode duration ended");
      alertMode = false;
      setField8(0);  // Reset field8 to 0 after alert mode ends
      delay(15000);
      return;  // Skip normal light cycle if in alert mode
    }
  }

  if(!alertMode) {
    // Normal light cycle
    if (currentLight == 0 && currentMillis - previousMillis >= redTime) {
      currentLight = 1; // Red to Green
      previousMillis = currentMillis;
      controlTrafficLight("red");
      sendData("0", "0", "1");
    } else if (currentLight == 1 && currentMillis - previousMillis >= greenTime) {
      currentLight = 2; // Green to Orange
      previousMillis = currentMillis;
      controlTrafficLight("green");
      sendData("1", "0", "0");
    } else if (currentLight == 2 && currentMillis - previousMillis >= orangeTime) {
      currentLight = 0; // Orange to Red
      previousMillis = currentMillis;
      controlTrafficLight("orange");
      sendData("0", "1", "0");
    }
  }
}
