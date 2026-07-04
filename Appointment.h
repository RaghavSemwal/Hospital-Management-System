#ifndef APPOINTMENT_H
#define APPOINTMENT_H
#include <string>

class Appointment {
public:
    int appointmentId;
    int patientId;
    int doctorId;
    std::string timeSlot;  
    bool feePaid;          
    std::string status;    

    Appointment(int aId, int pId, int dId, std::string slot, bool paid, std::string stat)
        : appointmentId(aId), patientId(pId), doctorId(dId), timeSlot(slot), feePaid(paid), status(stat) {}
};

#endif