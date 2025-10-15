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
program::program(int program_id, int current_step_id, int total_steps)
    : program_id(program_id), current_step_id(current_step_id), total_steps(total_steps)
{
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

