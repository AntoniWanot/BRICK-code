#ifndef SD_CARD_H
#define SD_CARD_H

#include <SD.h>
#include <ArduinoJson.h>
#include "program.h"

// Motor configuration structure
struct motor_config {
    int id;
    int cube_position;  // 0=FRONT, 1=TOP, 2=BOTTOM, 3=LEFT, 4=RIGHT
    int min_degrees;    // Minimum angle (negative = backward from zero)
    int max_degrees;    // Maximum angle (positive = forward from zero)
};

// Container for all motor configurations
struct all_motors_config {
    motor_config motors[6];  // Support up to 6 motors
    int motor_count;
    bool valid;
};

class sd_card
{
private:
    /* data */
public:
    
    sd_card(/* args */);
    String return_manifest();
    program load_program(int &program_id,int (&pins)[3][2]);
    
    // Config loading and parsing
    all_motors_config load_config();
    bool validate_motor_position(int position);
    bool validate_motor_angle(int motor_id, int angle, const all_motors_config &config);
    
    ~sd_card();
};

#endif // SD_CARD_H
