#include "sd_card.h"

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
  DynamicJsonDocument listed_files(4096);
  File file = SD.open("manifest.json");
  if (!file) {
    Serial.println("Error: manifest.json not found");
    return "";
  }
  DeserializationError err = deserializeJson(listed_files, file);
  file.close();
  if (err) {
    Serial.print("JSON parse error: ");
    Serial.println(err.c_str());
    return "";
  }

  String out;
  serializeJson(listed_files, out);      // compact JSON string
  out += "\nEND_OF_MANIFEST\n";         // sentinel
  return out;
}
program sd_card::load_program(int &program_id,int (&pins)[3][2]) {
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

    
    program current_program(program_id, current_step_load, total_steps, pins);
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

            current_joint new_joint(joint_id, joint_angle, joint_direction, pins[joint_id][0], pins[joint_id][1]);
        }
        current_step new_step(i);
        current_program.add_step(new_step);
    }
}
