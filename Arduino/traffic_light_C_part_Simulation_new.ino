#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Tsomi";
const char* password = "klapatsimpas123";
String api = "JEBAFQ64U1HT0JEQ"; // Write Thingspeak Channel API Key
String channelID = "2704086";

// Traffic light delays
int DELAY_GREEN = 30000;
int DELAY_YELLOW = 20000;
int DELAY_RED = 30000;

// LED pin setup
int greenLED = 14;  // Pin for green LED
int yellowLED = 12; // Pin for yellow LED
int redLED = 13;    // Pin for red LED

unsigned long lastFieldUpdate = 0; // Time of the last field update
unsigned long fieldCheckInterval = 600000; // 10 minutes in milliseconds

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  setField8("0"); // Initialize field8 to 0
}

void loop() {
  // // Check for keystrokes
  // if (Serial.available() > 0) {
  //   char input = Serial.read();
  //   if (input == 'e') {
  //     Serial.println("Keystroke detected! Changing field8 to 1.");
  //     setField8("1");
  //   }
  // }

  // Check the field value and act accordingly
  int fieldValue = getField8();
  if (fieldValue == 1) {
    // Yellow light stays on for 20 seconds
    controlTrafficLight("YELLOW");
    sendData("0", "1", "0");
    delayWithInterrupt(20000); // Non-blocking delay
    setField8("0"); // Reset field8 after interval
    delay(2000);
  } else {
    // Normal traffic light cycle
    normalTrafficLightCycle();
  }

  // Update the field every 10 minutes
  if (millis() - lastFieldUpdate >= fieldCheckInterval) {
    setField8("1");
    lastFieldUpdate = millis();
  }
}

void normalTrafficLightCycle() {
  controlTrafficLight("RED");
  sendData("0", "0", "1");
  delayWithInterrupt(DELAY_RED);

  controlTrafficLight("GREEN");
  sendData("1", "0", "0");
  delayWithInterrupt(DELAY_GREEN);

  controlTrafficLight("YELLOW");
  sendData("0", "1", "0");
  delayWithInterrupt(DELAY_YELLOW);
}

void sendData(String green, String yellow, String red) {
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field1=" + green + "&field2=" + yellow + "&field3=" + red;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error sending request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void controlTrafficLight(String state) {
  if (state == "GREEN") {
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
    Serial.println("Traffic Light: GREEN");
  } else if (state == "YELLOW") {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
    Serial.println("Traffic Light: YELLOW");
  } else if (state == "RED") {
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
    Serial.println("Traffic Light: RED");
  }
}

void delayWithInterrupt(int duration) {
  unsigned long startMillis = millis();
  while (millis() - startMillis < duration) {
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'e') {
        Serial.println("Keystroke detected during delay. Field8 changed to 1.");
        setField8("1");
        delay(2000);
        break;
      }
    }
  }
}

void setField8(String value) {
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field8=" + value;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("Field8 updated to: ");
      Serial.println(value);
    } else {
      Serial.print("Error updating field8: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

int getField8() {
  String url = "http://api.thingspeak.com/channels/" + channelID + "/fields/8.json?api_key=" + api + "&results=1";
  HTTPClient http;
  http.begin(url);

  int httpResponseCode = http.GET();
  if (httpResponseCode == 200) {
    String payload = http.getString();
    Serial.println("JSON Response: " + payload);

    String feedsSection = extractFeedsSection(payload);
    return extractField8Value(feedsSection);
  } else {
    Serial.print("Error reading field8: ");
    Serial.println(httpResponseCode);
    return -1;
  }
  http.end();
}

String extractFeedsSection(String payload) {
  int startIdx = payload.indexOf("\"feeds\":[");
  int endIdx = payload.indexOf("]}", startIdx);
  if (startIdx == -1 || endIdx == -1) {
    Serial.println("Error: 'feeds' section not found.");
    return "";
  }
  return payload.substring(startIdx + 8, endIdx + 1);
}

int extractField8Value(String feedsSection) {
  int field8Pos = feedsSection.indexOf("\"field8\":\"");
  if (field8Pos == -1) {
    Serial.println("Error: 'field8' value not found.");
    return -1;
  }
  String field8String = feedsSection.substring(field8Pos + 10, field8Pos + 11);
  return field8String.toInt();
}
