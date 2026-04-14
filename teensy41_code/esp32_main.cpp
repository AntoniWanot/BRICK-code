#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "config.h"
#include "my_webserver.h"
#include "communication.hpp"

communicator comm;
bool manifest_received = false;  // Flag to track if manifest has been received

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1);
  
  delay(500);  // Wait for serial to be ready
  Serial.println("\n[SETUP] === ESP32 Robot Controller Initialization ===");
  
  // Initialize GPIO pins for robot brick motors
  pinMode(MOTOR1_PLUS_PIN, OUTPUT);
  pinMode(MOTOR1_MINUS_PIN, OUTPUT);
  pinMode(MOTOR2_PLUS_PIN, OUTPUT);
  pinMode(MOTOR2_MINUS_PIN, OUTPUT);
  pinMode(MOTOR3_PLUS_PIN, OUTPUT);
  pinMode(MOTOR3_MINUS_PIN, OUTPUT);
  
  Serial.println("[SETUP] GPIO pins initialized");
  
  // Set all pins to LOW initially
  digitalWrite(MOTOR1_PLUS_PIN, LOW);
  digitalWrite(MOTOR1_MINUS_PIN, LOW);
  digitalWrite(MOTOR2_PLUS_PIN, LOW);
  digitalWrite(MOTOR2_MINUS_PIN, LOW);
  digitalWrite(MOTOR3_PLUS_PIN, LOW);
  digitalWrite(MOTOR3_MINUS_PIN, LOW);
  
  // Setup WiFi as Access Point
  Serial.println("[SETUP] Starting WiFi AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
  delay(100);
  
  Serial.println("[SETUP] WiFi AP Started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
  // Initialize web server
  Serial.println("[SETUP] Starting web server...");
  setupWebServer();
  delay(1000);
  Serial.println("[SETUP] Web server started");
  
  // Initialize communicator and receive manifest
  Serial.println("[SETUP] Sending ready signal...");
  comm.send_ready_signal();
  Serial.println("[SETUP] Ready signal sent");
  
  // Wait a moment for Teensy to respond
  delay(100);
  
  // Receive manifest from Teensy
  Serial.println("[SETUP] Waiting for manifest from Teensy...");
  if (comm.receive_manifest()) {
    manifest_received = true;
    Serial.println("[SETUP] ✓ Manifest received and parsed successfully");
  } else {
    Serial.println("[SETUP] ✗ Failed to receive manifest");
  }
  
  Serial.println("[SETUP] === Initialization Complete ===\n");
}

void loop() {
  // Main loop - server runs asynchronously
  // Manifest is received once during setup, not in the loop
  
  checkJogTimeout();
  checkStartTimeout();
  delay(10);
}

