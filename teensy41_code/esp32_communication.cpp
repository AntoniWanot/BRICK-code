#include "communication.hpp"

communicator::communicator() {
}

bool communicator::send_ready_signal() {
  Serial.println("[COMM] Sending READY signal");
  Serial2.println("READY");
  return true;
}
bool communicator::receive_manifest() {
  Serial.println("[COMM] Waiting for manifest (15s timeout)...");
  unsigned long start_time = millis();
  String response;
  int bytes_received = 0;
  
  while (millis() - start_time < 15000) {
    if (Serial2.available()) {
      char c = Serial2.read();
      response += c;
      bytes_received++;
      
      // Debug: print first few bytes
      if (bytes_received <= 50) {
        Serial.print("[COMM DEBUG] Byte ");
        Serial.print(bytes_received);
        Serial.print(": ");
        Serial.println(c);
      }
      
      if (response.indexOf("END_OF_MANIFEST") != -1) {
        Serial.println("[COMM] Found END_OF_MANIFEST marker");
        break;
      }
    } else {
      // Debug: show when Serial2 has no data
      if (bytes_received == 0 && (millis() - start_time) % 1000 == 0) {
        Serial.print("[COMM DEBUG] Waiting... elapsed: ");
        Serial.print(millis() - start_time);
        Serial.println("ms, Serial2.available() = false");
      }
    }
  }
  
  Serial.print("[COMM] Total bytes received: ");
  Serial.println(bytes_received);
  
  if (response.length() == 0) {
    Serial.println("[COMM] ERROR: No data received within timeout");
    Serial.println("[COMM] Troubleshooting:");
    Serial.println("  - Check TX/RX wiring between Teensy (TX8/RX8) and ESP32 (RX2/TX2)");
    Serial.println("  - Verify both use same baud rate (9600)");
    Serial.println("  - Check if Teensy is actually sending (check Teensy debug output)");
    return false;
  }

  int idx = response.indexOf("END_OF_MANIFEST");
  if (idx != -1) response = response.substring(0, idx);
  response.trim();

  Serial.print("[COMM] Manifest received (length: ");
  Serial.print(response.length());
  Serial.println(" bytes)");
  
  Serial.println("[COMM DEBUG] Manifest content (first 200 chars):");
  Serial.println(response.substring(0, 200));

  Serial2.println("MANIFEST_RECEIVED");
  return m_manifest.manifest_parser(response);
}

bool communicator::send_program_id(int &program_id) {
  Serial2.println(program_id);
  return true;
}

manifest::manifest() {
}

bool manifest::manifest_parser(const String &manifest_str) {
  programs.clear();
  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, manifest_str);
  if (err) {
    Serial.print("[MANIFEST] JSON parsing error: ");
    Serial.println(err.c_str());
    return false;
  }

  JsonArray arr = doc["programs"].as<JsonArray>();
  if (!arr) {
    Serial.println("[MANIFEST] ERROR: 'programs' array not found in manifest");
    return false;
  }

  Serial.print("[MANIFEST] Parsing ");
  Serial.print(arr.size());
  Serial.println(" programs...");

  for (JsonObject obj : arr) {
    Program_Entry e;
    e.id = obj["id"] | 0;
    e.name = String((const char*)(obj["name"] | ""));
    e.description = String((const char*)(obj["description"] | ""));
    e.created = String((const char*)(obj["created"] | ""));
    programs.push_back(e);
    
    Serial.print("[MANIFEST] Program: ID=");
    Serial.print(e.id);
    Serial.print(" Name=");
    Serial.println(e.name);
  }
  Serial.println("[MANIFEST] Parsing complete");
  return true;
}