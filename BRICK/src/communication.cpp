#include "communication.h"
bool send_manifest(const String &manifest)
{
  sd_card sd;
  String manifest = sd.return_manifest();
  Serial2.println(manifest);
  unsigned long start_time = millis();
  while (millis() - start_time < 15000) // Wait up to 15 seconds for response
  {
    if (Serial2.available())
    {
      String response = Serial2.readStringUntil('\n');
      if (response == "MANIFEST_RECEIVED")
      {
        return true;
      }
    }
  }
  return false;
}

int receive_program_id()
{
  while (!Serial2.available())
  {
    // Do nothing, just wait
  }
  int program_id = Serial2.parseInt();
  return program_id;
}
