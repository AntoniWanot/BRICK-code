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
    program load_program(int &program_id,int (&pins)[2][2]);
    ~sd_card();
};
