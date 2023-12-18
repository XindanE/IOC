

/*
const char* ssid = "tp";
const char* password = "xmyz02200059Y";
const char* mqttServer = "172.20.10.12";
const int mqttPort = 1883;
const char* mqttUser = "wz";
const char* mqttPassword = "123456";
*/
#include <WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16


// Wi-Fi
const char* ssid = "tp";
const char* password = "xmyz02200059Y";

//MQTT broker's IP address
const char* mqttServer = "172.20.10.12";
const int mqttPort = 1883;
const char* mqttUser = "wz";
const char* mqttPassword = "123456";

WiFiClient espClient;
PubSubClient client(espClient);

// Sensor and actuator pins
int photoResistorPin = 36;
int ledPin = 17;
float val = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  Wire.begin(4, 15); // pins SDA , SCL
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  display.drawPixel(10, 10, WHITE);

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  
  
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.println("Connected to Wi-Fi");

  // Connect to MQTT broker
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT broker...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.print("Failed to connect, error state: ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  
}

void loop() {

  // Read sensor data and publish to MQTT broker
  int photoResistorValue = analogRead(photoResistorPin);
  Serial.print("senor: ");
  Serial.println(photoResistorValue);
  delay(1000);
  
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  display.clearDisplay();      //clear
  val = 100.0 -(float(photoResistorValue) * 100.0 /4095.0 );
  display.print(val);            //print
  display.print('%');
  display.display();
  
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.println("Reconnecting to MQTT broker...");
      if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
        Serial.println("Connected to MQTT broker");
      } else {
        Serial.print("Failed to connect, error state: ");
        Serial.println(client.state());
        delay(2000);
      }
    }
  }
  
  client.loop();
  char msg[10];
  snprintf(msg, 10, "%d", photoResistorValue);
  client.publish("testTopic", msg);

  delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  if (String(topic) == "testTopic") {
    if (message == "ON") {
      digitalWrite(ledPin, HIGH);
    } else if (message == "OFF") {
      digitalWrite(ledPin, LOW);
    }
  }
}
