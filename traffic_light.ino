String ssid = "Simulator Wifi";
String password = "";
String host = "api.thingspeak.com";
const int httpPort = 80;
String greenUri = "/update?api_key=GL5G0DJHRPVAWIXA&field1=";
String orangeUri = "/update?api_key=GL5G0DJHRPVAWIXA&field2=";
String redUri = "/update?api_key=GL5G0DJHRPVAWIXA&field3=";

// Constants to represent light colors
const int GREEN = 10;
const int ORANGE = 9;
const int RED = 8;

int currentLightState = GREEN; // Initially set to green

int setupESP8266(void) {
  Serial.begin(115200);
  Serial.println("AT");
  delay(10);
  if (!Serial.find("OK")) return 1;

  // Set WiFi mode to station
  Serial.println("AT+CWMODE=1");
  delay(2000);
  if (!Serial.find("OK")) return 2;

  // Connect to WiFi
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  Serial.println(cmd);
  delay(5000); // Wait a bit for the connection to establish
  if (!Serial.find("OK")) return 3;

  // Check IP address (optional, for verification)
  Serial.println("AT+CIFSR");
  delay(2000);
  if (!Serial.find("OK")) return 4;

  // Connect to ThingSpeak
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(5000); // Wait for connection to establish
  if (!Serial.find("OK")) return 5;

  Serial.println("WiFi Connected and ThingSpeak API ready");
  return 0; // Success
}

void sendLightState(int lightState) {
  String uri;
  
  // Choose the right field based on the current light state
  if (lightState == GREEN) {
    uri = greenUri + String(1); // Sending 1 to represent "Green Light ON" in field1
  } else if (lightState == ORANGE) {
    uri = orangeUri + String(1); // Sending 1 to represent "Orange Light ON" in field2
  } else if (lightState == RED) {
    uri = redUri + String(1); // Sending 1 to represent "Red Light ON" in field3
  }

  // Send the HTTP request
  String httpPacket = "GET " + uri + " HTTP/1.1\r\nHost: " + host + "\r\n";
  int length = httpPacket.length();

  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10);

  Serial.print(httpPacket);
  delay(10);
  if (!Serial.find("SEND OK\r\n")) return;

  Serial.println("Light state sent: " + String(lightState));
}

void cycleTrafficLight() {
  // Simulate the cycling of traffic lights: green -> orange -> red
  if (currentLightState == GREEN) {
    currentLightState = ORANGE;
  } else if (currentLightState == ORANGE) {
    currentLightState = RED;
  } else if (currentLightState == RED) {
    currentLightState = GREEN;
  }
}

void anydata() {
  // Send the current traffic light state
  sendLightState(currentLightState);
  cycleTrafficLight(); // Move to the next light state after sending
}

void setup() {
  setupESP8266();
}

void loop() {
  anydata();
  delay(10000); // Wait 10 seconds between sending data
}
