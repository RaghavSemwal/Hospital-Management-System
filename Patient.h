#ifndef PATIENT_H
#define PATIENT_H
#include <string>

class Patient {
public:
    int id;
    std::string name;
    int age;
    std::string ailment;

    Patient(int id, std::string name, int age, std::string ailment)
        : id(id), name(name), age(age), ailment(ailment) {}
};
#endif