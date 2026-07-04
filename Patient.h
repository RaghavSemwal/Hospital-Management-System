#ifndef PATIENT_H
#define PATIENT_H
#include <string>

class Patient {
public:
    int id;
    std::string name;
    int age;
    std::string ailment;
    std::string phoneNumber; 

    Patient(int id, std::string name, int age, std::string ailment, std::string phone)
        : id(id), name(name), age(age), ailment(ailment), phoneNumber(phone) {}
};

#endif