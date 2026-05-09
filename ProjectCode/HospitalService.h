#ifndef HOSPITAL_SERVICE_H
#define HOSPITAL_SERVICE_H

#include <vector>

#include "Appointment.h"
#include "HospitalRepository.h"
#include "Patient.h"

class HospitalService {
private:
    std::vector<Patient> patients;
    std::vector<Appointment> appointments;
    int nextPatientId;
    int nextAppointmentId;

public:
    HospitalService();

    void initialize(HospitalRepository& repository);
    std::vector<Patient> getPatients() const;
    std::vector<Appointment> getAppointmentsForPatient(int patientId) const;
    Patient registerPatient(const std::string& name, int age, HospitalRepository& repository);
    Appointment scheduleAppointment(int patientId, const std::string& date, const std::string& time, HospitalRepository& repository);
    bool updatePatient(int patientId, const std::string& name, int age, HospitalRepository& repository);
    bool cancelAppointment(int appointmentId, HospitalRepository& repository);
    bool patientExists(int patientId) const;
    bool appointmentExists(int appointmentId) const;
    bool isTimeSlotAvailable(const std::string& date, const std::string& time) const;
    bool validatePatientInput(const std::string& name, int age) const;
    bool validateAppointmentInput(const std::string& date, const std::string& time) const;
};

#endif
