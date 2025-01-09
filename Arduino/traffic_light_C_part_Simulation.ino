#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Tsomi";
const char* password = "klapatsimpas123";
String api = "JEBAFQ64U1HT0JEQ"; // Write Thingspeak Channel API Key
String sec_api = "IGWF3BB80UPS74HJ"; // Write Thingspeak Channel API Key for the other app
String channelID = "2704086";

// Traffic light delays
int DELAY_GREEN = 30000;
int DELAY_YELLOW = 20000;
int DELAY_RED = 30000;
String tr_light = "";

// LED pin setup
int greenLED = 14;  // Pin for green LED
int yellowLED = 12; // Pin for orange LED
int redLED = 13;   // Pin for red LED

void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi...");

  // Set up LED pins
  pinMode(greenLED, OUTPUT);
  pinMode(yellowLED, OUTPUT);
  pinMode(redLED, OUTPUT);

  // WiFi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  
  // Initialize field8 to 0
  setField8("0");
}

void loop() {
  // Traffic light control
  tr_light = "RED";
  controlTrafficLight(tr_light);
  sendData("0", "0", "1");
  delayWithInterrupt(DELAY_RED);

  tr_light = "GREEN";
  controlTrafficLight(tr_light);
  sendData("1", "0", "0");
  delayWithInterrupt(DELAY_GREEN);

  tr_light = "YELLOW";
  controlTrafficLight(tr_light);
  sendData("0", "1", "0");
  delayWithInterrupt(DELAY_YELLOW);

}

void sendData(String green, String orange, String red) {
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field1=" + green + "&field2=" + orange + "&field3=" + red;
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error on sending request: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

void delayWithInterrupt(int duration) {
  unsigned long startMillis = millis();
  unsigned long yellowLightStartMillis = 0;
  bool yellowLightOn = false;

  while (millis() - startMillis < duration) {
    // Check for serial input during the delay
    if (Serial.available() > 0) {
      char input = Serial.read();
      if (input == 'e') {
        Serial.println("\nKeystroke detected! Changing field8 to 1.");
        setField8("1");
        delay(1000); // Allow time for the change to propagate

        yellowLightStartMillis = millis();
        yellowLightOn = true;
        
        while (yellowLightOn && (millis() - yellowLightStartMillis < 20000)) { // 20 seconds
          // Keep the light on YELLOW for 20 seconds
          controlTrafficLight("YELLOW");
          sendData("0", "1", "0");
          delay(500); // Check periodically
        }

        // After 20 seconds, reset field8 and resume normal cycle
        Serial.println("Field8 reset to 0. Resuming normal cycle.");
        setField8("0");
        yellowLightOn = false;
      }
    }

    // Normal delay without interrupt
    if (!yellowLightOn) {
      delay(100); // Small delay to prevent overwhelming the loop
    }
  }
}

void controlTrafficLight(String state) {
  if (state == "GREEN") {
    Serial.println("Traffic Light: GREEN");
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (state == "YELLOW") {
    Serial.println("Traffic Light: YELLOW");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, HIGH);
    digitalWrite(redLED, LOW);
  } else if (state == "RED") {
    Serial.println("Traffic Light: RED");
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
    digitalWrite(redLED, HIGH);
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
    Serial.println("JSON Response: ");
    Serial.println(payload);  

    String feedsSection = extractFeedsSection(payload);
    int field8Value = extractField8Value(feedsSection);
    
    return field8Value;
  } else {
    Serial.print("Error on sending request: ");
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
