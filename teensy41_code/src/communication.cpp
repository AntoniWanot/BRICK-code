#include "communication.h"
bool send_manifest()
{
  sd_card sd;
  String manifest = sd.return_manifest();
  Serial8.println(manifest);
  Serial.println(manifest);
  unsigned long start_time = millis();
  while (millis() - start_time < 15000) // Wait up to 15 seconds for response
  {
    if (Serial8.available())
    {
      String response = Serial8.readStringUntil('\n');
      if (response == "MANIFEST_RECEIVED")
      {
        return true;
      }
    }
  }
  return false;
}
bool wait_for_ready_signal()
{
  unsigned long start_time = millis();
  while (millis() - start_time < 5000) // Wait up to 5 seconds for response
  {
    if(digitalRead(READY_SIGNAL) == HIGH) 
    {
      return true;
    }
  }
  return false;
}
int receive_program_id()
{
  while (!Serial8.available())
  {
    // Do nothing, just wait
  }
  int program_id = Serial8.parseInt();
  return program_id;
}
