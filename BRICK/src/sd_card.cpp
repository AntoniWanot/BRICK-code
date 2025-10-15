#include <SD.h>
#include <ArduinoJson.h>
#include "program.h"
class sd_card
{
private:
    /* data */
public:
    sd_card(/* args */);
    String return_manifest();
    void load_program(int program_id);
    ~sd_card();
};

sd_card::sd_card(/* args */)
{
    
    if (!SD.begin()) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        return;
    }
    Serial.println("card initialized.");
}

sd_card::~sd_card()
{
}

String sd_card::return_manifest() {
    String manifest = "";
    File file = SD.open("manifest.json");
    JsonDocument listed_files;
    deserializeJson(listed_files, file);
    JsonArray programs=listed_files["programs"];

    for (JsonObject program : programs) {
        manifest += "ID: " + String(program["id"]) + "\n";
        manifest += "Name: " + String(program["name"]) + "\n";
        manifest += "File: " + String(program["file"]) + "\n";
        manifest += "Description: " + String(program["description"]) + "\n";
        manifest += "Created: " + String(program["created"]) + "\n";
        manifest += "\n";
    }

    return manifest;
}
void sd_card::load_program(int program_id) {
    File file = SD.open("manifest.json");
    JsonDocument listed_files;
    deserializeJson(listed_files, file);
    JsonArray programs=listed_files["programs"];

    String filename = "";
    for (JsonObject program : programs) {
        if (program["id"] == program_id) {
            filename = String(program["file"]);
            break;
        }
    }
    File program_file = SD.open(filename.c_str());
    JsonDocument programs_listed_files;
    deserializeJson(programs_listed_files, program_file);
    
    JsonObject programObj = programs_listed_files["program"];
    if (!programObj) {
        Serial.println("Error: no program object in file");
        return;
    }

    int current_step_load = programObj["current_step"] | 0;
    int total_steps = programObj["total_steps"] | 0;
    JsonArray steps = programObj["steps"];

    
    program current_program(program_id, current_step_load, total_steps);
    for (JsonObject step : steps)
    {
        JsonArray joints = step["joints"];
        int i = 0;
        for (size_t j = 0; j < joints.size(); j++)
        {
            JsonObject joint = joints[j];
            i++;
            int joint_id = joint["joint"] | 0;
            int joint_angle = joint["angle"] | 0;
            bool joint_direction;
            if (joint["direction"] == 1)
            {
                joint_direction = true;
            }
            else
            {
                joint_direction = false;
            }

            current_joint new_joint(joint_id, joint_angle, joint_direction);
        }
        current_step new_step(i);
        current_program.add_step(new_step);
    }
}
