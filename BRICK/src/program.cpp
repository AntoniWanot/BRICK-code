#include "program.h"

// current_step implementations
current_joint::current_joint()
    : joint(0), angle(0), direction(true), current_step_count(0)
{
}

current_joint::current_joint(int joint, int angle, bool direction, int step_pin, int dir_pin)
    : joint(joint), angle(angle), direction(direction), step_pin(step_pin), dir_pin(dir_pin), current_step_count(0)
{
    steps_to_move = amount_of_steps();
    if(direction)
    {
        digitalWrite(dir_pin, HIGH);
    }
    else
    {
        digitalWrite(dir_pin, LOW);
    }
}

current_joint::~current_joint()
{
}


int current_joint::amount_of_steps()
{
    // Configuration constants - set these once based on your hardware
    const int MICROSTEPS_PER_REV = 800; // Change to 1600 if using that setting
    const int GEAR_RATIO = 49;
    
    double steps_per_degree = (MICROSTEPS_PER_REV * GEAR_RATIO) / 360.0;
    double steps_tmp = angle * steps_per_degree;
    int steps = static_cast<int>(std::round(steps_tmp));
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
bool current_step::execute()
{
    // Check if there are any joints to execute
    if (joints.empty()) {
        return false;
    }
    
    // Initialize all joint step counters to 0
    for (auto& joint : joints) {
        joint.current_step_count = 0;
    }
    
    bool any_joint_active = true;
    
    // Execute all joints in parallel until all are finished
    while (any_joint_active) {
        any_joint_active = false;
        
        // Execute one step for each joint that hasn't finished
        for (auto& joint : joints) {
            if (joint.execute_one_step()) {
                any_joint_active = true;
            }
        }
        
        // Small delay between steps for proper stepper motor timing
        // Adjust this delay based on your stepper motor requirements
        delayMicroseconds(1000); // 1ms delay, adjust as needed
    }
    
    return true; // All joints completed their movement
}

bool current_step::is_completed() const
{
    // Check if all joints in this step are completed
    for (const auto& joint : joints) {
        if (!joint.is_completed()) {
            return false;
        }
    }
    return true;
}
bool current_joint::execute_one_step()
{
    if(current_step_count >= steps_to_move)
        return false;
    
    // Generate step pulse
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(2); // Minimum pulse width for most stepper drivers (2-5 microseconds)
    digitalWrite(step_pin, LOW);
    delayMicroseconds(2); // Minimum time between pulses
    
    current_step_count++;
    return true;
}

bool current_joint::is_completed() const
{
    return current_step_count >= steps_to_move;
}

// program implementations
program::program(int &program_id, int &current_step_id, int &total_steps, int (&pins_to)[3][2])
    : program_id(program_id), current_step_id(current_step_id), total_steps(total_steps)
{
    pins[0][0] = pins_to[0][0];
    pins[0][1] = pins_to[0][1];
    pins[1][0] = pins_to[1][0];
    pins[1][1] = pins_to[1][1];
    pins[2][0] = pins_to[2][0];
    pins[2][1] = pins_to[2][1];
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
    if (current_step_id >= total_steps)
        return false;

    current_step &step = steps[current_step_id];
    
    // Execute the current step (all joints in parallel)
    bool step_completed = step.execute();
    
    if (step_completed) {
        current_step_id++; // Move to next step
        
        // Check if we've completed all steps
        if (current_step_id >= total_steps) {
            return false; // Program completed
        }
    }
    
    return true; // Program still running
}

bool program::is_completed() const
{
    return current_step_id >= total_steps;
}