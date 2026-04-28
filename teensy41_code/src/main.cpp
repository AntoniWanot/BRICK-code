#include <Arduino.h>
#include <SoftwareSerial.h>
#include "config.h"
#include "sd_card.h"
#include "program.h"
#include "communication.h"
#include "manual_mover.hpp"


int pins_output[3][2]={{MOTOR1_STEP_PIN, MOTOR1_DIR_PIN}, {MOTOR2_STEP_PIN, MOTOR2_DIR_PIN}, {MOTOR3_STEP_PIN, MOTOR3_DIR_PIN}}; // Step and Dir pins for three joints

// JOG input pin configuration [PLUS, MINUS] for each motor
int jog_pins[3][2] = {
  {IN_MOTOR1_PLUS_PIN, IN_MOTOR1_MINUS_PIN},   // Motor 1
  {IN_MOTOR2_PLUS_PIN, IN_MOTOR2_MINUS_PIN},   // Motor 2
  {IN_MOTOR3_PLUS_PIN, IN_MOTOR3_MINUS_PIN}    // Motor 3
};

// Create ManualMover instance
ManualMover manual_mover(pins_output, jog_pins);

int program_id_received = -1;
unsigned long millis_begin = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(12000);
  Serial8.begin(9600, SERIAL_8N1);
  
  delay(100); // Brief delay for serial initialization
  Serial.println("\n========== TEENSY 4.1 ROBOT CONTROLLER ==========");
  Serial.println("Initializing system...");
  //initialize input pins
  pinMode(READY_SIGNAL, INPUT);
  // Initialize motor output pins
  pinMode(pins_output[0][0], OUTPUT);
  pinMode(pins_output[0][1], OUTPUT);
  pinMode(pins_output[1][0], OUTPUT);
  pinMode(pins_output[1][1], OUTPUT);
  pinMode(pins_output[2][0], OUTPUT);
  pinMode(pins_output[2][1], OUTPUT);
  // Set initial state of motor pins to LOW
  for (int i = 0; i < 3; i++) {
    digitalWrite(pins_output[i][0], LOW);
    digitalWrite(pins_output[i][1], LOW);
  }
  
  Serial.println("[✓] Motor output pins configured (3 motors × 2 pins)");
  
  // Initialize ManualMover
  manual_mover.init();
  Serial.println("[✓] ManualMover (JOG control) initialized");
  
  // Send manifest and wait for confirmation

  Serial.println("[→] Waiting for ready signal from controller...");
  while (wait_for_ready_signal());
  Serial.println("[✓] Ready signal received");
  Serial.println("[→] Sending manifest to controller...");
  send_manifest();
  Serial.println("[✓] Manifest sent");
  Serial.println("================================================\n");
  
}

void loop() {
  if (Serial8.available()) {
    program_id_received = receive_program_id();
    Serial.print("[✓] Program ID received: ");
    Serial.println(program_id_received);
    if (program_id_received > 0) {
      Serial.println("[✓] Valid program ID received");
    } else {
      Serial.println("[!] Invalid program ID received");
    }
  }


  
  if (program_id_received > 0)
  {
    sd_card sd;
    program program_current = sd.load_program(program_id_received, pins_output);
    Serial.println("[✓] Program loaded successfully");
    Serial.print("[*] Total steps in program: ");
    Serial.println(program_current.total_steps);
    
    // Execute the program step by step
    Serial.println("[⚡] Starting program execution...\n");
    unsigned long start_time = millis();
    int step_count = 0;
    
    while (!program_current.is_completed()) {
      if (!program_current.run()) {
        Serial.println("[!] Program execution error or completed");
        break;
      }
      
      step_count++;
      if (step_count % 1 == 0) {  // Report every step
        Serial.print("[*] Step: ");
        Serial.print(program_current.current_step_id);
        Serial.print(" / ");
        Serial.println(program_current.total_steps);
      }
      
      

      
      // Optional: Add a small delay between steps if needed
      // delay(10);
    }
  
    unsigned long execution_time = millis() - start_time;
    Serial.println("\n================================================");
    Serial.println("[✓] Program execution completed successfully");
    Serial.print("[*] Execution time: ");
    Serial.print(execution_time);
    Serial.println(" ms");
    Serial.println("================================================\n");
  }
  // Check for manual JOG input between program steps
    manual_mover.check_inputs();
    manual_mover.perform_step();
}

