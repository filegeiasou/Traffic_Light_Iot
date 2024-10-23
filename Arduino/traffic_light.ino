String ssid = "Simulator Wifi";
String password = "";
String host = "api.thingspeak.com";
const int httpPort = 80;
String api = "YOUR_API_KEY";

// LED pin setup (adjust according to your pinout)
int greenLED = 10; // Pin for green LED
int orangeLED = 9; // Pin for orange LED
int redLED = 8; // Pin for red LED

int setupESP8266(void){
  Serial.begin(115200);
  Serial.println("AT");
  delay(10);
  if(!Serial.find("OK")) return 1;
  
  // Set WiFi mode to station
  Serial.println("AT+CWMODE=1");
  delay(2000);
  if(!Serial.find("OK")) return 2;

  // Connect to WiFi
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  Serial.println(cmd);
  delay(5000); // Wait a bit for the connection to establish
  if(!Serial.find("OK")) return 3;

  // Check IP address (optional, for verification)
  Serial.println("AT+CIFSR");
  delay(2000);
  if(!Serial.find("OK")) return 4;

  // Connect to ThingSpeak
  Serial.println("AT+CIPSTART=\"TCP\",\"" + host + "\"," + httpPort);
  delay(5000); // Wait for connection to establish
  if(!Serial.find("OK")) return 5;

  Serial.println("WiFi Connected and ThingSpeak API ready");
  return 0; // Success
}

void setup() {
  // Setup the ESP8266 and LEDs
  pinMode(greenLED, OUTPUT);
  pinMode(orangeLED, OUTPUT);
  pinMode(redLED, OUTPUT);
  
  setupESP8266();
}

// Thingspeak: Field1 = Green, Field2 = Orange, Field3 = Red
void sendData(String green, String orange, String red) {

  // Construct and send the HTTP GET request
  String url = "/update?api_key=" + api + "&field1=" + green + "&field2=" + orange + "&field3=" + red;
  String httpPacket = "GET " + uri + " HTTP/1.1\r\nHost: " + String(host) + "\r\nConnection: close\r\n\r\n";
  int length = httpPacket.length();
  
  Serial.print("AT+CIPSEND=");
  Serial.println(length);
  delay(10);
  
  Serial.print(httpPacket);
  delay(10);
  if(!Serial.find("SEND OK\r\n")) return;
}

void controlTrafficLight(String state) {
  if (state == "green") {
    digitalWrite(greenLED, HIGH);
    digitalWrite(orangeLED, LOW);
    digitalWrite(redLED, LOW);
  } else if (state == "orange") {
    digitalWrite(greenLED, LOW);
    digitalWrite(orangeLED, HIGH);
    digitalWrite(redLED, LOW);
  } else if (state == "red") {
    digitalWrite(greenLED, LOW);
    digitalWrite(orangeLED, LOW);
    digitalWrite(redLED, HIGH);
  }
}

void loop() {
  // Simulate traffic light control
  controlTrafficLight("red");
  sendData("0", "0", "1");
  delay(5000);

  controlTrafficLight("green");
  sendData("1", "0", "0");
  delay(5000);

  controlTrafficLight("orange");
  sendData("0", "1", "0");
  delay(2000);
}
