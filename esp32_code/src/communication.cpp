#include "communication.hpp"
bool communicator::send_ready_signal() {
  Serial2.println("READY");
  return true;
}
bool communicator::receive_manifest() {
  unsigned long start_time = millis();
  String response;
  while (millis() - start_time < 15000) {
    while (Serial2.available()) {
      response += (char)Serial2.read();
      if (response.indexOf("END_OF_MANIFEST") != -1) break;
    }
    if (response.indexOf("END_OF_MANIFEST") != -1) break;
  }
  if (response.length() == 0) return false;

  int idx = response.indexOf("END_OF_MANIFEST");
  if (idx != -1) response = response.substring(0, idx);
  response.trim();

  Serial2.println("MANIFEST_RECEIVED");
  return manifest.manifest_parser(response);
}

bool communicator::send_program_id(int &program_id) {
  Serial2.println(program_id);
  return true;
}

manifest::manifest() {
    manifest = "";
}

bool manifest::manifest_parser(const String &manifest_str) {
  programs.clear();
  DynamicJsonDocument doc(4096);
  DeserializationError err = deserializeJson(doc, manifest_str);
  if (err) {
    return false;
  }

  JsonArray arr = doc["programs"].as<JsonArray>();
  if (!arr) return false;

  for (JsonObject obj : arr) {
    Program_Entry e;
    e.id = obj["id"] | 0;
    e.name = String((const char*)(obj["name"] | ""));
    e.description = String((const char*)(obj["description"] | ""));
    e.created = String((const char*)(obj["created"] | ""));
    programs.push_back(e);
  }
  return true;
}