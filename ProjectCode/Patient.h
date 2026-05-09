#ifndef PATIENT_H
#define PATIENT_H

#include <string>

struct Patient {
    int id;
    std::string name;
    int age;

    Patient() : id(0), age(0) {}
    Patient(int patientId, const std::string& patientName, int patientAge)
        : id(patientId), name(patientName), age(patientAge) {}
};

#endif
