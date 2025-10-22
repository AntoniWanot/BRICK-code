#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "sd_card.h"
bool send_manifest(const String &manifest);
int receive_program_id();
#endif // COMMUNICATION_H
