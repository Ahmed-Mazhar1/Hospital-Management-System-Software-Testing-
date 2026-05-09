#include "HospitalService.h"

#include <cctype>
#include <stdexcept>

namespace {
bool isDigits(const std::string& value) {
    if (value.empty()) {
        return false;
    }
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(value[i]))) {
            return false;
        }
    }
    return true;
}
}

HospitalService::HospitalService() : nextPatientId(1), nextAppointmentId(1) {}

void HospitalService::initialize(HospitalRepository& repository) {
    patients = repository.loadPatients();
    appointments = repository.loadAppointments();
    nextPatientId = 1;
    nextAppointmentId = 1;

    for (std::size_t i = 0; i < patients.size(); ++i) {
        if (patients[i].id >= nextPatientId) {
            nextPatientId = patients[i].id + 1;
        }
    }
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].id >= nextAppointmentId) {
            nextAppointmentId = appointments[i].id + 1;
        }
    }
}

std::vector<Patient> HospitalService::getPatients() const {
    return patients;
}

std::vector<Appointment> HospitalService::getAppointmentsForPatient(int patientId) const {
    std::vector<Appointment> matches;
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].patientId == patientId) {
            matches.push_back(appointments[i]);
        }
    }
    return matches;
}

Patient HospitalService::registerPatient(const std::string& name, int age, HospitalRepository& repository) {
    if (!validatePatientInput(name, age)) {
        throw std::runtime_error("Invalid patient information.");
    }

    Patient patient(nextPatientId++, name, age);
    patients.push_back(patient);
    repository.saveAll(patients, appointments);
    return patient;
}

Appointment HospitalService::scheduleAppointment(int patientId, const std::string& date, const std::string& time, HospitalRepository& repository) {
    if (!patientExists(patientId)) {
        throw std::runtime_error("Patient ID does not exist.");
    }
    if (!validateAppointmentInput(date, time)) {
        throw std::runtime_error("Invalid date or time format.");
    }
    if (!isTimeSlotAvailable(date, time)) {
        throw std::runtime_error("Appointment slot is already taken.");
    }

    Appointment appointment(nextAppointmentId++, patientId, date, time);
    appointments.push_back(appointment);
    repository.saveAll(patients, appointments);
    return appointment;
}

bool HospitalService::updatePatient(int patientId, const std::string& name, int age, HospitalRepository& repository) {
    if (!validatePatientInput(name, age)) {
        return false;
    }

    for (std::size_t i = 0; i < patients.size(); ++i) {
        if (patients[i].id == patientId) {
            patients[i].name = name;
            patients[i].age = age;
            repository.saveAll(patients, appointments);
            return true;
        }
    }
    return false;
}

bool HospitalService::cancelAppointment(int appointmentId, HospitalRepository& repository) {
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].id == appointmentId) {
            appointments.erase(appointments.begin() + static_cast<std::vector<Appointment>::difference_type>(i));
            repository.saveAll(patients, appointments);
            return true;
        }
    }
    return false;
}

bool HospitalService::patientExists(int patientId) const {
    for (std::size_t i = 0; i < patients.size(); ++i) {
        if (patients[i].id == patientId) {
            return true;
        }
    }
    return false;
}

bool HospitalService::appointmentExists(int appointmentId) const {
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].id == appointmentId) {
            return true;
        }
    }
    return false;
}

bool HospitalService::isTimeSlotAvailable(const std::string& date, const std::string& time) const {
    for (std::size_t i = 0; i < appointments.size(); ++i) {
        if (appointments[i].date == date && appointments[i].time == time) {
            return false;
        }
    }
    return true;
}

bool HospitalService::validatePatientInput(const std::string& name, int age) const {
    return !name.empty() && age > 0 && age < 130;
}

bool HospitalService::validateAppointmentInput(const std::string& date, const std::string& time) const {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
        return false;
    }
    if (time.size() != 5 || time[2] != ':') {
        return false;
    }

    if (!isDigits(date.substr(0, 4)) || !isDigits(date.substr(5, 2)) || !isDigits(date.substr(8, 2))) {
        return false;
    }
    if (!isDigits(time.substr(0, 2)) || !isDigits(time.substr(3, 2))) {
        return false;
    }

    int month = std::atoi(date.substr(5, 2).c_str());
    int day = std::atoi(date.substr(8, 2).c_str());
    int hour = std::atoi(time.substr(0, 2).c_str());
    int minute = std::atoi(time.substr(3, 2).c_str());

    return month >= 1 && month <= 12 &&
           day >= 1 && day <= 31 &&
           hour >= 0 && hour <= 23 &&
           minute >= 0 && minute <= 59;
}
