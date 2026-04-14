#ifndef COMMUNICATION_H
#define COMMUNICATION_H
#include <Arduino.h>
#include <vector>
#include <ArduinoJson.h>
struct Program_Entry
{
    int id;
    String name;
    String description;
    String created;
};
class manifest{
    public:
        manifest();
        bool manifest_parser(const String &manifest_str);
        std::vector<Program_Entry> programs; 
};
class communicator {
    public:
        communicator();
        bool send_ready_signal();
        bool receive_manifest();
        bool send_program_id(int &program_id);
    private:
        manifest m_manifest;
};
#endif // COMMUNICATION_H