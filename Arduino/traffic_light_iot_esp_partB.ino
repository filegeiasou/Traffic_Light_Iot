#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Tsomi";
const char* password = "sidarta123";
String api = "RVYMKEOS5DUCTSNQ"; // Traffic Api Key
String sec_api = "NNF1B9GGD4OMXA1M";

// LED pin setup (adjust according to your pinout)
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
void setField8(String value, String api) {
  // Construct and send the HTTP GET request
  String url = "http://api.thingspeak.com/update?api_key=" + api + "&field8=" + value;
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

void loop() {
  // Simulate traffic light control
  controlTrafficLight("red");
  sendData("0", "0", "1");
  delay(20000);

  controlTrafficLight("green");
  sendData("1", "0", "0");
  delay(20000);

  controlTrafficLight("orange");
  sendData("0", "1", "0");
  delay(15000);
}