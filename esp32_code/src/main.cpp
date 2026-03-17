#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "my_webserver.h"
#define RXD2 16
#define TXD2 17
void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  
  // Initialize GPIO pins for robot brick motors
  pinMode(START_PIN, OUTPUT);
  pinMode(MOTOR1_PLUS_PIN, OUTPUT);
  pinMode(MOTOR1_MINUS_PIN, OUTPUT);
  pinMode(MOTOR2_PLUS_PIN, OUTPUT);
  pinMode(MOTOR2_MINUS_PIN, OUTPUT);
  pinMode(MOTOR3_PLUS_PIN, OUTPUT);
  pinMode(MOTOR3_MINUS_PIN, OUTPUT);
  
  // Set all pins to LOW initially
  digitalWrite(START_PIN, LOW);
  digitalWrite(MOTOR1_PLUS_PIN, LOW);
  digitalWrite(MOTOR1_MINUS_PIN, LOW);
  digitalWrite(MOTOR2_PLUS_PIN, LOW);
  digitalWrite(MOTOR2_MINUS_PIN, LOW);
  digitalWrite(MOTOR3_PLUS_PIN, LOW);
  digitalWrite(MOTOR3_MINUS_PIN, LOW);
  
  // Setup WiFi as Access Point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  
  Serial.println("WiFi AP Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Initialize web server
  setupWebServer();
  
  Serial.println("Web server started");
}

void loop() {
  // Main loop - server runs asynchronously
  while (!Serial2.available())();
  
  checkJogTimeout();
  checkStartTimeout();
  delay(10); // Krótkie opóźnienie aby nie obciążać procesora
}
