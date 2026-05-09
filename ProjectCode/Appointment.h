#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <string>

struct Appointment {
    int id;
    int patientId;
    std::string date;
    std::string time;

    Appointment() : id(0), patientId(0) {}
    Appointment(int appointmentId, int patient, const std::string& appointmentDate, const std::string& appointmentTime)
        : id(appointmentId), patientId(patient), date(appointmentDate), time(appointmentTime) {}
};

#endif
