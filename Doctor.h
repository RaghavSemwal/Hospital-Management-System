#ifndef DOCTOR_H
#define DOCTOR_H
#include <string>

class Doctor {
public:
    int id;
    std::string name;
    std::string specialization;

    // Default constructor (required for vector initializations by teammates)
    Doctor() : id(0), name(""), specialization("") {}

    // Parameterized constructor
    Doctor(int id, std::string name, std::string specialization)
        : id(id), name(name), specialization(specialization) {}
};

#endif