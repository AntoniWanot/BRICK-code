// program.h

#ifndef PROGRAM_H
#define PROGRAM_H

#include <vector>
#include <cstddef>

// Represents a single step for a motor/joint
class current_joint
{
public:
    int joint;
    int angle;
    bool direction; // true is clockwise, false is counterclockwise

    current_joint();
    current_joint(int joint, int angle, bool direction);
    ~current_joint();
};
class current_step
{
public:
    int number_of_joints = 0;
    std::vector<current_joint> joints;

    current_step();
    void add_joint(const current_joint &j);
    current_step(int number_of_joints);
    ~current_step();
};

// Represents a program composed of multiple steps
class program
{
public:
    int program_id = 0;
    int current_step_id = 0;
    int total_steps = 0;

    // Container holding the program steps in order
    std::vector<current_step> steps;

    program(int program_id, int current_step_id, int total_steps);
    ~program();

    // Add a step to the program
    void add_step(const current_step &s);

    // Number of steps currently stored
    std::size_t step_count() const;

    // Access a step by index (returns nullptr if out of range)
    current_step *get_step(std::size_t idx);
    const current_step *get_step(std::size_t idx) const;

    // Iterators to walk steps
    std::vector<current_step>::iterator begin();
    std::vector<current_step>::iterator end();
};

#endif // PROGRAM_H
