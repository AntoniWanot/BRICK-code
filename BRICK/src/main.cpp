#include <Arduino.h>
#include <SoftwareSerial.h>
#include "sd_card.h"
#include "program.h"
#include "communication.h"
int pins[2][2]={{24,25},{26,27}}; // Step and Dir pins for two joints
//int Step_J1 = 24; // Stepper motor for Joint 1
//int Dir_J1 = 25;  // Direction for Joint 1
//int Step_J2 = 26; // Stepper motor for Joint 2
//int Dir_J2 = 27;  // Direction for Joint 2
#define RXD2 16
#define TXD2 17
String manifest = "";

int program_id_received = -1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(12000);
  Serial2.begin(9600, SERIAL_8N1);
  pinMode(pins[0][0], OUTPUT);
  pinMode(pins[0][1], OUTPUT);
  pinMode(pins[1][0], OUTPUT);
  pinMode(pins[1][1], OUTPUT);
// Send manifest and wait for confirmation
  send_manifest(manifest);
}

void loop() {
  // Wait for program ID
  while (!Serial2.available())
  {
    // Do nothing, just wait
  }
  program_id_received = receive_program_id();
  sd_card sd;
  program program_current = sd.load_program(program_id_received,pins);
  program_current.run();

  
  
  
  

}

