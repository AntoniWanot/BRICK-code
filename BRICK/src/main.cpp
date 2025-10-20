#include <Arduino.h>
#include <SoftwareSerial.h>
#include "sd_card.h"
#include "program.h"
#define Step_J1 24 // Stepper motor for Joint 1
#define Dir_J1 25  // Direction for Joint 1
#define Step_J2 26 // Stepper motor for Joint 2
#define Dir_J2 27  // Direction for Joint 2
#define Step_J3 28 // Stepper motor for Joint 3
#define Dir_J3 29  // Direction for Joint 3
#define RXD2 16
#define TXD2 17 
String manifest="";

bool send_manifest();
int receive_program_id();
int program_id_received = -1;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(12000);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  pinMode(Step_J1, OUTPUT);
  pinMode(Dir_J1, OUTPUT);
  pinMode(Step_J2, OUTPUT);
  pinMode(Dir_J2, OUTPUT);
  pinMode(Step_J3, OUTPUT);
  pinMode(Dir_J3, OUTPUT);
// Send manifest and wait for confirmation

  send_manifest();
  program_id_received = receive_program_id();
}

void loop() {
  // Wait for program ID
  while (!Serial2.available())
  {
    // Do nothing, just wait
  }


  if (Serial2.available())
  {
    int program_id = Serial2.parseInt();
    sd_card sd;
    sd.load_program(program_id);
  }
  
  

}

bool send_manifest()
{
  sd_card sd;
  manifest = sd.return_manifest();
  Serial2.println(manifest);
  unsigned long start_time = millis();
  while (millis() - start_time < 15000) // Wait up to 15 seconds for response
  {
    if (Serial2.available())
    {
      String response = Serial2.readStringUntil('\n');
      if (response == "MANIFEST_RECEIVED")
      {
        return true;
      }
    }
  }
  return false;
}

int receive_program_id()
{
  while (!Serial2.available())
  {
    // Do nothing, just wait
  }
  int program_id = Serial2.parseInt();
  return program_id;
}