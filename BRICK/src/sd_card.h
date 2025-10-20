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
