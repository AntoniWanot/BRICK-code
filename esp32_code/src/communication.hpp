#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <Arduino.h>
#include <vector>
struct Program_Entry
{
    int id;
    String name;
    String description;
    String created;
};

class communicator {
    public:
        bool send_ready_signal();
        String receive_manifest();
        bool send_program_id(int &program_id);
};
class manifest{
    public:
        bool manifest_parser(const String &manifest_str);
        std::vector<Program_Entry> programs; 
}
#endif // COMMUNICATION_H