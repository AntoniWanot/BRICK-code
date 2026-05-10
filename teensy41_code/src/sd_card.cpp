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
  if (listed_files["programs"].is<JsonArray>()) {
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
  
  return out;  // Just send JSON, no extra sentinel
}
program sd_card::load_program(int &program_id,int (&pins)[3][2]) {
    int step_id = 0, total_steps_init = 0;  // Local variables for error returns
    
    File file = SD.open("manifest.json");
    if (!file) {
        Serial.println("[!] ERROR: Failed to open manifest.json");
        return program(program_id, step_id, total_steps_init, pins);
    }
    JsonDocument listed_files;
    deserializeJson(listed_files, file);
    file.close();
    JsonArray programs=listed_files["programs"];

    String filename = "";
    for (JsonObject program_entry : programs) {
        if (program_entry["id"] == program_id) {
            filename = String(program_entry["file"]);
            break;
        }
    }
    
    if (filename.length() == 0) {
        Serial.print("[!] ERROR: Program ID ");
        Serial.print(program_id);
        Serial.println(" not found in manifest");
        return program(program_id, step_id, total_steps_init, pins);
    }
    
    File program_file = SD.open(filename.c_str());
    if (!program_file) {
        Serial.print("[!] ERROR: Could not open program file: ");
        Serial.println(filename);
        return program(program_id, step_id, total_steps_init, pins);
    }
    
    JsonDocument programs_listed_files;
    DeserializationError err = deserializeJson(programs_listed_files, program_file);
    program_file.close();
    
    if (err) {
        Serial.print("[!] JSON parse error: ");
        Serial.println(err.c_str());
        return program(program_id, step_id, total_steps_init, pins);
    }
    
    JsonObject programObj = programs_listed_files["program"];
    if (!programObj) {
        Serial.println("[!] Error: no program object in file");
        return program(program_id, step_id, total_steps_init, pins);
    }

    int current_step_load = programObj["current_step"] | 0;
    int total_steps = programObj["total_steps"] | 0;
    JsonArray steps = programObj["steps"];

    program current_program(program_id, current_step_load, total_steps, pins);
    for (JsonObject step : steps)
    {
        JsonArray joints = step["joints"];
        current_step new_step;  // Don't pre-allocate; use default constructor
        
        for (size_t j = 0; j < joints.size(); j++)
        {
            JsonObject joint = joints[j];
            int joint_id = joint["joint"] | 0;
            
            // Bounds check: joint_id should be 1-3, which maps to array indices 0-2
            if (joint_id < 1 || joint_id > 3) {
                Serial.print("[!] WARNING: Invalid joint ID ");
                Serial.print(joint_id);
                Serial.println(" in program, skipping");
                continue;
            }
            
            // Read angle as float first, then convert to int (JSON stores as 90.0, 45.0, etc.)
            float angle_float = joint["angle"] | 0.0;
            int joint_angle = (int)angle_float;
            bool joint_direction = (joint["direction"] == 1) ? true : false;
            
            // Debug: print what was read
            Serial.print("[DEBUG] Joint ID: ");
            Serial.print(joint_id);
            Serial.print(", Angle: ");
            Serial.print(joint_angle);
            Serial.print(", Direction: ");
            Serial.println(joint_direction);
            
            // Array index is joint_id - 1 (since joint_id is 1-3, not 0-2)
            int pin_index = joint_id - 1;
            current_joint new_joint(joint_id, joint_angle, joint_direction, pins[pin_index][0], pins[pin_index][1]);
            new_step.add_joint(new_joint);
        }
        current_program.add_step(new_step);
    }
    
    return current_program;
}
bool sd_card::validate_motor_position(int position) {
    // Valid positions: 0=FRONT, 1=TOP, 2=BOTTOM, 3=LEFT, 4=RIGHT (5=BACK not usable)
    return (position >= 0 && position <= 4);
}

bool sd_card::validate_motor_angle(int motor_id, int angle, const all_motors_config &config) {
    // Find motor configuration
    for (int i = 0; i < config.motor_count; i++) {
        if (config.motors[i].id == motor_id) {
            return (angle >= config.motors[i].min_degrees && angle <= config.motors[i].max_degrees);
        }
    }
    return false; 
}

all_motors_config sd_card::load_config() {
    all_motors_config result = {{}, 0, false};
    
    File file = SD.open("config.json");
    if (!file) {
        Serial.println("[!] ERROR: Could not open config.json");
        return result;
    }
    
    JsonDocument config;
    DeserializationError err = deserializeJson(config, file);
    file.close();
    
    if (err) {
        Serial.print("[!] JSON parse error in config.json: ");
        Serial.println(err.c_str());
        return result;
    }

    JsonVariant motors_var = config["motors"];
    if (!motors_var.is<JsonArray>()) {
        Serial.println("[!] ERROR: 'motors' key not found or not an array in config.json");
        return result;
    }
    JsonArray motors_array = motors_var.as<JsonArray>();
    
    int motor_count = 0;
    Serial.println("[→] Parsing motor configurations from config.json:");
    
    for (JsonObject motor_obj : motors_array) {
        if (motor_count >= 6) {
            Serial.println("[!] WARNING: Maximum 6 motors supported, ignoring additional motors");
            break;
        }
        
        int motor_id = motor_obj["id"] | -1;
        int cube_position = motor_obj["cube_position"] | -1;
        
        // Extract limits
        JsonObject limits = motor_obj["limits"];
        int min_deg = limits["min_degrees"] | 0;
        int max_deg = limits["max_degrees"] | 0;
        
        // Validate cube position
        if (!validate_motor_position(cube_position)) {
            Serial.print("[!] WARNING: Invalid cube_position ");
            Serial.print(cube_position);
            Serial.print(" for motor ");
            Serial.print(motor_id);
            Serial.println(" (valid: 0-4)");
            continue;
        }
        
        // Validate angle limits
        if (min_deg >= max_deg) {
            Serial.print("[!] WARNING: Invalid angle limits for motor ");
            Serial.print(motor_id);
            Serial.print(" (min ");
            Serial.print(min_deg);
            Serial.print(" >= max ");
            Serial.print(max_deg);
            Serial.println(")");
            continue;
        }
        
        // Store motor config
        result.motors[motor_count].id = motor_id;
        result.motors[motor_count].cube_position = cube_position;
        result.motors[motor_count].min_degrees = min_deg;
        result.motors[motor_count].max_degrees = max_deg;
        
        // Debug output
        Serial.print("[✓] Motor ");
        Serial.print(motor_id);
        Serial.print(" - Position: ");
        Serial.print(cube_position);
        Serial.print(", Angle limits: ");
        Serial.print(min_deg);
        Serial.print("° to ");
        Serial.print(max_deg);
        Serial.println("°");
        
        motor_count++;
    }
    
    result.motor_count = motor_count;
    result.valid = (motor_count > 0);
    
    if (result.valid) {
        Serial.print("[✓] Configuration loaded successfully: ");
        Serial.print(motor_count);
        Serial.println(" motors configured");
    } else {
        Serial.println("[!] ERROR: No valid motors found in config.json");
    }
    
    return result;
}
