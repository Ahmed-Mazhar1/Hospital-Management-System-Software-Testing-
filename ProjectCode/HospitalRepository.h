#ifndef HOSPITAL_REPOSITORY_H
#define HOSPITAL_REPOSITORY_H

#include <string>
#include <vector>

#include "Appointment.h"
#include "Patient.h"

class HospitalRepository {
private:
    std::string filePath;

public:
    explicit HospitalRepository(const std::string& filePath = "hospital.json");

    std::vector<Patient> loadPatients() const;
    std::vector<Appointment> loadAppointments() const;
    void saveAll(const std::vector<Patient>& patients, const std::vector<Appointment>& appointments) const;
    bool fileExists() const;
};

#endif
