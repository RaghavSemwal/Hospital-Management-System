#ifndef DOCTOR_H
#define DOCTOR_H
#include <string>

class Doctor {
public:
    int id;
    std::string name;
    std::string specialization;

    Doctor(int id, std::string name, std::string specialization)
        : id(id), name(name), specialization(specialization) {}
};
#endif