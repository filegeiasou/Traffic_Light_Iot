#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Tsomi";
const char* password = "klapatsimpas123";
String api = "JEBAFQ64U1HT0JEQ"; // Write Thingspeak Channel API Key
String read_api = "IGWF3BB80UPS74HJ"; // Read API key for ThingSpeak
String channel_id = "2704086"; // Channel ID for ThingSpeak

// LED Pins
int greenLED = 14;
int yellowLED = 12;
int redLED = 13;

// Traffic light timings
unsigned long redTime = 30000;    // 30 seconds
unsigned long greenTime = 30000; // 30 seconds
unsigned long orangeTime = 20000; // 20 seconds
unsigned long alertDuration = 20000; // Alert lasts 20 seconds

// Variables
unsigned long previousMillis = 0;     // Tracks last light change
unsigned long alertStartTime = 0;     // Tracks start of alert mode
unsigned long lastField8Update = 0;   // Last field8 update time
unsigned long lastCheckMillis = 0;    // Last ThingSpeak check time
unsigned long checkInterval = 5000;  // Check ThingSpeak every 5 seconds
unsigned long updateInterval = 5000; // Send updates every 5 seconds
int currentLight = 0;                 // 0 = Red, 1 = Green, 2 = Orange
bool alertMode = false;               // Alert mode state

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
}

void controlTrafficLight(String state) {
  if (state == "red") {
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);
    digitalWrite(yellowLED, LOW);
  } else if (state == "green") {
    digitalWrite(redLED, LOW);
    digitalWrite(greenLED, HIGH);
    digitalWrite(yellowLED, LOW);
  } else if (state == "orange") {
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
      Serial.print("Field 8 Value: ");
      Serial.println(payload);
      http.end();
      delay(100); // Delay to stabilize communication
      return payload.toInt();
    } else {
      Serial.println("Error reading ThingSpeak");
      http.end();
      delay(100); // Delay to handle failed read
      return -1;
    }
  }
  return -1;
}

void setField8(int value) {
  unsigned long currentMillis = millis();

  // Throttle updates to avoid flooding ThingSpeak
  if (currentMillis - lastField8Update >= updateInterval) {
    String url = "http://api.thingspeak.com/update?api_key=" + api + "&field8=" + String(value);
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(url);
      int httpResponseCode = http.GET();

      if (httpResponseCode > 0) {
        Serial.print("Updated Field 8 to: ");
        Serial.println(value);
        delay(500); // Allow time for the ThingSpeak channel to process the update
      } else {
        Serial.println("Error updating Field 8");
        delay(500); // Handle error gracefully with a delay
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }
    lastField8Update = currentMillis;
  }
}

void loop() {
  unsigned long currentMillis = millis();

  // Periodically check ThingSpeak field
  if (currentMillis - lastCheckMillis >= checkInterval) {
    lastCheckMillis = currentMillis;
    int fieldValue = getField8();
    if (fieldValue == 1 && !alertMode) {
      alertMode = true;
      alertStartTime = currentMillis; // Start alert mode
      Serial.println("Entered Alert Mode via ThingSpeak!");
    }
  }

  // Check for keystroke to enter alert mode
  if (Serial.available() > 0) {
    char input = Serial.read();
    if (input == 'e' && !alertMode) {
      alertMode = true;
      alertStartTime = currentMillis; // Start alert mode
      setField8(1); // Update field8 to 1
      Serial.println("Entered Alert Mode via Keystroke!");
    }
  }

  // Handle alert mode
  if (alertMode) {
    controlTrafficLight("orange");
    // setField8(1); // Ensure field8 is 1 during alert mode
    delay(20000);
    alertMode = false; // Exit alert mode
    setField8(0);      // Reset field8 to 0
    Serial.println("Exiting Alert Mode");
    // if (currentMillis - alertStartTime >= alertDuration) { // Alert lasts 20 seconds
    //   alertMode = false; // Exit alert mode
    //   setField8(0);      // Reset field8 to 0
    //   Serial.println("Exiting Alert Mode");
    // }
    return; // Skip normal light cycle
  }

  // Normal light cycle
  if (currentLight == 0 && currentMillis - previousMillis >= redTime) {
    currentLight = 1; // Red to Green
    previousMillis = currentMillis;
    controlTrafficLight("green");
  } else if (currentLight == 1 && currentMillis - previousMillis >= greenTime) {
    currentLight = 2; // Green to Orange
    previousMillis = currentMillis;
    controlTrafficLight("orange");
  } else if (currentLight == 2 && currentMillis - previousMillis >= orangeTime) {
    currentLight = 0; // Orange to Red
    previousMillis = currentMillis;
    controlTrafficLight("red");
  }
}
