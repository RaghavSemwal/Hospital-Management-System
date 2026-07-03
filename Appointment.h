#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include <string>

class Appointment {
public:
    int appointmentId;
    int patientId;
    int doctorId;
    std::string date;

    Appointment(int aId, int pId, int dId, std::string dt)
        : appointmentId(aId), patientId(pId), doctorId(dId), date(dt) {}
};
#endif