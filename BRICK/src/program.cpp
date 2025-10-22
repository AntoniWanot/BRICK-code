#include "program.h"

// current_step implementations
current_joint::current_joint()
    : joint(0), angle(0), direction(true)
{
}

current_joint::current_joint(int joint, int angle, bool direction)
    : joint(joint), angle(angle), direction(direction)
{
}

current_joint::~current_joint()
{
}

int current_joint::amount_of_steps()
{
    double steps_tmp = 0;
    steps_tmp=angle/1.8;
    steps_tmp=steps_tmp*49;
    int steps=static_cast<int>(steps_tmp);
    return steps;
}

current_step::current_step()
    : number_of_joints(0), joints()
{
}
current_step::current_step(int number_of_joints)
    : number_of_joints(number_of_joints), joints(number_of_joints)
{
}
void current_step::add_joint(const current_joint &j)
{
    joints.push_back(j);
    number_of_joints = static_cast<int>(joints.size());
}
current_step::~current_step()
{
}

// program implementations
program::program(int &program_id, int &current_step_id, int &total_steps, int (&pins_to)[2][2])
    : program_id(program_id), current_step_id(current_step_id), total_steps(total_steps)
{
    pins[0][0] = pins_to[0][0];
    pins[0][1] = pins_to[0][1];
    pins[1][0] = pins_to[1][0];
    pins[1][1] = pins_to[1][1];
}

program::~program()
{
}

void program::add_step(const current_step &s)
{
    steps.push_back(s);
    total_steps = static_cast<int>(steps.size());
}


std::size_t program::step_count() const
{
    return steps.size();
}


current_step *program::get_step(std::size_t idx)
{
    if (idx >= steps.size())
        return nullptr;
    return &steps[idx];
}


const current_step *program::get_step(std::size_t idx) const
{
    if (idx >= steps.size())
        return nullptr;
    return &steps[idx];
}


std::vector<current_step>::iterator program::begin()
{
    return steps.begin();
}


std::vector<current_step>::iterator program::end()
{
    return steps.end();
}

bool program::run()
{
    for (int step_index = 0; step_index < total_steps; step_index++)
    {
        current_step *step = get_step(step_index);
        if (!step)
            return false;
//i want to move all joints in a step simultaneously but idk how to do that with stepper motors :(
        for (int joint_index = 0; joint_index < step->number_of_joints; joint_index++)
        {
            current_joint &joint = step->joints[joint_index];
            int step_pin = pins[joint.joint][0];
            int dir_pin = pins[joint.joint][1];

            digitalWrite(dir_pin, joint.direction ? HIGH : LOW);
            int steps_to_move = joint.amount_of_steps();
            for (int s = 0; s < steps_to_move; s++)
            {

//i forgor how to setup frequency i will check at home :3
                digitalWrite(step_pin, HIGH);
                
                digitalWrite(step_pin, LOW);
                delayMicroseconds(1000); // Interval between steps
            }
        }
    }
    return true;
}