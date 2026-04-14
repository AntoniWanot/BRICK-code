#include "sd_card.h"

// Static flag to track if SD card has been initialized
static bool sd_initialized = false;

sd_card::sd_card(/* args */)
{
    // Only initialize once
    if (sd_initialized) {
        Serial.println("[→] SD card already initialized");
        return;
    }
    
    Serial.println("[→] Attempting to initialize SD card with BUILTIN_SDCARD...");
    delay(500); // Give SD card time to be ready
    
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("[!] ERROR: SD card failed, or not present");
        Serial.println("[!] Possible causes:");
        Serial.println("    - SD card not inserted");
        Serial.println("    - SD card not formatted properly");
        Serial.println("    - Hardware connection issue");
        return;
    }
    
    sd_initialized = true;
    Serial.println("[✓] SD card initialized successfully");
    
    // Check if manifest.json exists
    if (SD.exists("manifest.json")) {
        Serial.println("[✓] manifest.json found on SD card");
    } else {
        Serial.println("[!] Warning: manifest.json NOT found on SD card");
    }
}

sd_card::~sd_card()
{
}

String sd_card::return_manifest() {
  JsonDocument listed_files;
  File file = SD.open("manifest.json");
  
  if (!file) {
    Serial.println("[!] ERROR: Could not open manifest.json");
    Serial.println("[!] Available files on SD card:");
    
    // List files on SD card for debugging
    File root = SD.open("/");
    File entry = root.openNextFile();
    int file_count = 0;
    while (entry) {
      Serial.print("    - ");
      Serial.println(entry.name());
      file_count++;
      entry = root.openNextFile();
    }
    root.close();
    
    if (file_count == 0) {
      Serial.println("    (SD card appears empty)");
    }
    
    return "";
  }
  
  // Get file size
  size_t file_size = file.size();
  Serial.print("[→] Reading manifest.json (size: ");
  Serial.print(file_size);
  Serial.println(" bytes)");
  
  // Read raw content for debugging
  String raw_content = "";
  while (file.available()) {
    raw_content += (char)file.read();
  }
  file.close();
  
  Serial.println("[DEBUG] Raw manifest content:");
  Serial.println(raw_content);
  
  if (raw_content.length() == 0) {
    Serial.println("[!] ERROR: manifest.json is empty!");
    return "";
  }
  
  // Reset file and parse JSON
  file = SD.open("manifest.json");
  DeserializationError err = deserializeJson(listed_files, file);
  file.close();
  
  if (err) {
    Serial.print("[!] JSON parse error: ");
    Serial.println(err.c_str());
    Serial.print("[!] Input was: ");
    Serial.println(raw_content);
    return "";
  }

  Serial.println("[✓] Manifest parsed successfully");
  
  // Debug: show what was parsed
  Serial.println("[DEBUG] JSON structure:");
  if (listed_files.containsKey("programs")) {
    size_t program_count = listed_files["programs"].size();
    Serial.print("[DEBUG] Found ");
    Serial.print(program_count);
    Serial.println(" programs");
  } else {
    Serial.println("[!] WARNING: 'programs' key not found in JSON!");
  }
  
  String out;
  serializeJson(listed_files, out);      // compact JSON string
  
  Serial.print("[DEBUG] Serialized output length: ");
  Serial.println(out.length());
  Serial.print("[DEBUG] Serialized content: ");
  Serial.println(out);
  
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
