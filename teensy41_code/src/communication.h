#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include "sd_card.h"
bool send_manifest();
int receive_program_id();
bool wait_for_ready_signal();
#endif // COMMUNICATION_H
