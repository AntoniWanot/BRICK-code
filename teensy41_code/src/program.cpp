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
    // Configuration constants set these once based on hardware configuration
    const int MICROSTEPS_PER_REV = 1600; 
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
    unsigned long total_iterations = 0;
    unsigned long progress_interval = 5000; // Report every 5000 iterations
    
    // Execute all joints in parallel until all are finished
    while (any_joint_active) {
        any_joint_active = false;
        
        // Execute one step for each joint that hasn't finished
        for (auto& joint : joints) {
            if (joint.execute_one_step()) {
                any_joint_active = true;
            }
        }
        
        total_iterations++;
        if (total_iterations % progress_interval == 0) {
            Serial.print("[*] Progress: ");
            Serial.print(total_iterations);
            Serial.println(" iterations");
        }
        
        // Minimal delay between step sets for stepper motor timing
        // Reduced from 50µs to 2µs for faster execution
        delayMicroseconds(2);
    }
    
    Serial.print("[✓] Step completed: ");
    Serial.print(total_iterations);
    Serial.println(" total iterations");
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
    
    // Generate step pulse - optimized for speed
    digitalWrite(step_pin, HIGH);
    delayMicroseconds(1); // Pulse width reduced to 1µs (stepper drivers typically need 1-2µs)
    digitalWrite(step_pin, LOW);
    delayMicroseconds(1); // Minimum time between pulses
    
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