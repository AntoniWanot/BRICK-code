#include "communication.h"
bool send_manifest()
{
  sd_card sd;
  String manifest = sd.return_manifest();
  
  // Send manifest with newline
  Serial8.println(manifest);
  Serial.println("[COMM] Manifest sent to ESP32");
  Serial.println(manifest);
  
  // Wait for acknowledgment from ESP32
  unsigned long start_time = millis();
  const unsigned long ACK_TIMEOUT = 15000; // 5 second timeout for acknowledgment
  
  while (millis() - start_time < ACK_TIMEOUT) {
    if (Serial8.available()) {
      String response = Serial8.readStringUntil('\n');
      response.trim();
      if (response == "MANIFEST_RECEIVED") {
        Serial.println("[COMM] ✓ MANIFEST_RECEIVED acknowledgment from ESP32");
        return true;
      }
    }
    delay(10); // Small delay to avoid busy waiting
  }
  
  Serial.println("[COMM] ✗ Timeout waiting for MANIFEST_RECEIVED acknowledgment");
  return false;
}
bool wait_for_ready_signal()
{
  unsigned long start_time = millis();
  const unsigned long READY_TIMEOUT = 15000; // 15 second timeout
  
  while (millis() - start_time < READY_TIMEOUT) {
    // Check serial for "READY" message from ESP32
    if (Serial8.available()) {
      String message = Serial8.readStringUntil('\n');
      message.trim();
      if (message == "READY") {
        Serial.println("[COMM] Received READY from ESP32");
        return true;
      }
    }
    delay(10); // Small delay to avoid busy waiting
  }
  
  Serial.println("[COMM] Timeout waiting for READY signal");
  return false;
}
int receive_program_id()
{
  unsigned long start_time = millis();
  const unsigned long PROGRAM_ID_TIMEOUT = 10000; // 10 second timeout
  
  while (millis() - start_time < PROGRAM_ID_TIMEOUT) {
    if (Serial8.available()) {
      int program_id = Serial8.parseInt();
      Serial.print("[COMM] Program ID received: ");
      Serial.println(program_id);
      return program_id;
    }
    delay(10); // Small delay to avoid busy waiting
  }
  
  Serial.println("[COMM] Timeout waiting for program ID");
  return -1; // Return -1 to indicate timeout
}
