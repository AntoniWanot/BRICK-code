#include "manual_mover.hpp"

ManualMover::ManualMover(int (*motor_pins)[2], int jog_input_pins[3][2])
    : pins_output(motor_pins), last_jog_time(0) {
  // Copy JOG pin configuration
  for (int i = 0; i < 3; i++) {
    jog_pins[i][0] = jog_input_pins[i][0];  // PLUS pin
    jog_pins[i][1] = jog_input_pins[i][1];  // MINUS pin
    motor_jog_active[i] = false;
  }
}

void ManualMover::init() {
  // Configure JOG input pins
  for (int i = 0; i < 3; i++) {
    pinMode(jog_pins[i][0], INPUT_PULLUP);  // PLUS pin
    pinMode(jog_pins[i][1], INPUT_PULLUP);  // MINUS pin
  }
  
  Serial.println("ManualMover initialized");
  Serial.print("JOG Pins - Motor 1: ");
  Serial.print(jog_pins[0][0]);
  Serial.print("/");
  Serial.println(jog_pins[0][1]);
  Serial.print("JOG Pins - Motor 2: ");
  Serial.print(jog_pins[1][0]);
  Serial.print("/");
  Serial.println(jog_pins[1][1]);
  Serial.print("JOG Pins - Motor 3: ");
  Serial.print(jog_pins[2][0]);
  Serial.print("/");
  Serial.println(jog_pins[2][1]);
}

void ManualMover::check_inputs() {
  // Check each motor's JOG inputs
  for (int motor = 0; motor < 3; motor++) {
    bool plus_pressed = (digitalRead(jog_pins[motor][0]) == LOW);
    bool minus_pressed = (digitalRead(jog_pins[motor][1]) == LOW);
    
    // Handle JOG logic
    if (plus_pressed && !minus_pressed) {
      // JOG+ pressed
      if (!motor_jog_active[motor]) {
        // Set direction to clockwise (HIGH)
        digitalWrite(pins_output[motor][1], HIGH);
        motor_jog_active[motor] = true;
        Serial.print("Motor ");
        Serial.print(motor + 1);
        Serial.println(" JOG+ started");
      }
    } else if (minus_pressed && !plus_pressed) {
      // JOG- pressed
      if (!motor_jog_active[motor]) {
        // Set direction to counter-clockwise (LOW)
        digitalWrite(pins_output[motor][1], LOW);
        motor_jog_active[motor] = true;
        Serial.print("Motor ");
        Serial.print(motor + 1);
        Serial.println(" JOG- started");
      }
    } else {
      // Neither pressed or both pressed - stop JOG
      if (motor_jog_active[motor]) {
        motor_jog_active[motor] = false;
        Serial.print("Motor ");
        Serial.print(motor + 1);
        Serial.println(" JOG stopped");
      }
    }
  }
}

void ManualMover::perform_step() {
  unsigned long currentTime = micros();
  
  // Execute JOG steps if timing interval has passed
  if (currentTime - last_jog_time >= JOG_STEP_INTERVAL) {
    bool step_taken = false;
    
    // Execute steps for active motors
    for (int motor = 0; motor < 3; motor++) {
      if (motor_jog_active[motor]) {
        digitalWrite(pins_output[motor][0], HIGH);  // STEP pulse HIGH
        step_taken = true;
      }
    }
    
    if (step_taken) {
      delayMicroseconds(JOG_STEP_DELAY);
      
      // Bring all active STEP pins LOW
      for (int motor = 0; motor < 3; motor++) {
        if (motor_jog_active[motor]) {
          digitalWrite(pins_output[motor][0], LOW);
        }
      }
      
      last_jog_time = currentTime;
    }
  }
}

bool ManualMover::is_jogging() {
  for (int i = 0; i < 3; i++) {
    if (motor_jog_active[i]) return true;
  }
  return false;
}

void ManualMover::stop_all() {
  for (int i = 0; i < 3; i++) {
    motor_jog_active[i] = false;
  }
  Serial.println("All JOG movements stopped");
}
