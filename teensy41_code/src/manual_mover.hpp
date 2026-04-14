#ifndef MANUAL_MOVER_HPP
#define MANUAL_MOVER_HPP

#include <Arduino.h>

class ManualMover {
private:
  // Motor pin configuration (array of [STEP, DIR] pairs)
  int (*pins_output)[2];
  
  // JOG input pins for each motor [PLUS, MINUS]
  int jog_pins[3][2];
  
  // Motor state tracking
  bool motor_jog_active[3];
  unsigned long last_jog_time;
  
  // Timing parameters
  const unsigned long JOG_STEP_INTERVAL = 1000;  // 1ms between steps for smooth JOG
  const int JOG_STEP_DELAY = 200;                 // 200μs pulse width
  
  // Helper function to execute a single step for a motor
  void step_motor(int motor_index);

public:
  // Constructor - takes reference to motor pin array and JOG pin array
  ManualMover(int (*motor_pins)[2], int jog_input_pins[3][2]);
  
  // Initialize JOG pins (call in setup())
  void init();
  
  // Check JOG inputs and update direction/active states
  void check_inputs();
  
  // Execute JOG steps based on timing
  void perform_step();
  
  // Check if any motor is currently in JOG mode
  bool is_jogging();
  
  // Stop all JOG movements
  void stop_all();
};

#endif
