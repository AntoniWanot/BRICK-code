#ifndef CONFIG_H
#define CONFIG_H
//inputs
#define IN_MOTOR1_PLUS_PIN 36 // motor 1 JOG+ 2 esp32
#define IN_MOTOR1_MINUS_PIN 37// motor 1 JOG- 4 esp32
#define IN_MOTOR2_PLUS_PIN 38 // motor 2 JOG+ 5 esp32
#define IN_MOTOR2_MINUS_PIN 39// motor 2 JOG-  23 esp32
#define IN_MOTOR3_PLUS_PIN 40 // motor 3 JOG+ 19 esp32
#define IN_MOTOR3_MINUS_PIN 41// motor 3 JOG- 21 esp32

//outputs
#define MOTOR1_STEP_PIN 2
#define MOTOR1_DIR_PIN 3
#define MOTOR2_STEP_PIN 4
#define MOTOR2_DIR_PIN 5
#define MOTOR3_STEP_PIN 6
#define MOTOR3_DIR_PIN 7
#define RXD2 34
#define TXD2 35
#define STEPS_PER_OUTPUT_REV 78400 // 1600 steps/rev * 49 (gear ratio)
#endif
