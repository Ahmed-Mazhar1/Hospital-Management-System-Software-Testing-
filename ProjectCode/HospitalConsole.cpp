#include "HospitalConsole.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
bool parseInt(const std::string& text, int& value) {
    std::istringstream input(text);
    input >> value;
    return !input.fail() && input.eof();
}
}

HospitalConsole::HospitalConsole(HospitalService& hospitalService) : service(hospitalService) {}

void HospitalConsole::showMenu() const {
    std::cout << "\n--- Hospital Management System ---\n";
    std::cout << "1. Register Patient\n";
    std::cout << "2. Schedule Appointment\n";
    std::cout << "3. View Patient Records\n";
    std::cout << "4. Update Patient Information\n";
    std::cout << "5. Cancel Appointment\n";
    std::cout << "6. Exit\n";
}

void HospitalConsole::registerPatient() {
    HospitalRepository repository;
    std::string name = readString("Enter patient name: ");
    int age = readInt("Enter patient age: ");

    try {
        Patient patient = service.registerPatient(name, age, repository);
        std::cout << "Patient registered successfully with ID " << patient.id << ".\n";
    } catch (const std::exception& error) {
        std::cout << error.what() << "\n";
    }
}

void HospitalConsole::scheduleAppointment() {
    HospitalRepository repository;
    int patientId = readInt("Enter patient ID: ");
    std::string date = readString("Enter date (YYYY-MM-DD): ");
    std::string time = readString("Enter time (HH:MM): ");

    try {
        Appointment appointment = service.scheduleAppointment(patientId, date, time, repository);
        std::cout << "Appointment scheduled successfully with ID " << appointment.id << ".\n";
    } catch (const std::exception& error) {
        std::cout << error.what() << "\n";
    }
}

void HospitalConsole::viewPatientRecords() {
    std::vector<Patient> patients = service.getPatients();
    if (patients.empty()) {
        std::cout << "No patient records found.\n";
        return;
    }

    for (std::size_t i = 0; i < patients.size(); ++i) {
        std::cout << "ID: " << patients[i].id
                  << "  Name: " << patients[i].name
                  << "  Age: " << patients[i].age << "\n";

        std::vector<Appointment> appointments = service.getAppointmentsForPatient(patients[i].id);
        if (appointments.empty()) {
            std::cout << "  Appointments: none\n";
        } else {
            for (std::size_t j = 0; j < appointments.size(); ++j) {
                std::cout << "  Appointment ID: " << appointments[j].id
                          << "  Date: " << appointments[j].date
                          << "  Time: " << appointments[j].time << "\n";
            }
        }
    }
}

void HospitalConsole::updatePatientInformation() {
    HospitalRepository repository;
    int patientId = readInt("Enter patient ID to update: ");
    std::string name = readString("Enter updated name: ");
    int age = readInt("Enter updated age: ");

    if (service.updatePatient(patientId, name, age, repository)) {
        std::cout << "Patient information updated successfully.\n";
    } else {
        std::cout << "Update failed. Check the patient ID and input values.\n";
    }
}

void HospitalConsole::cancelAppointment() {
    HospitalRepository repository;
    int appointmentId = readInt("Enter appointment ID to cancel: ");

    if (service.cancelAppointment(appointmentId, repository)) {
        std::cout << "Appointment canceled successfully.\n";
    } else {
        std::cout << "Cancellation failed. Invalid appointment ID.\n";
    }
}

int HospitalConsole::readInt(const std::string& prompt) const {
    for (;;) {
        std::cout << prompt;
        std::string line;
        if (!std::getline(std::cin, line)) {
            return 0;
        }

        int value = 0;
        if (parseInt(line, value)) {
            return value;
        }
        std::cout << "Invalid number. Try again.\n";
    }
}

std::string HospitalConsole::readString(const std::string& prompt) const {
    std::cout << prompt;
    std::string value;
    std::getline(std::cin, value);
    return value;
}

void HospitalConsole::run() {
    HospitalRepository repository;
    service.initialize(repository);

    for (;;) {
        showMenu();
        int choice = readInt("Select option: ");

        if (choice == 1) {
            registerPatient();
        } else if (choice == 2) {
            scheduleAppointment();
        } else if (choice == 3) {
            viewPatientRecords();
        } else if (choice == 4) {
            updatePatientInformation();
        } else if (choice == 5) {
            cancelAppointment();
        } else if (choice == 6) {
            std::cout << "Goodbye.\n";
            break;
        } else {
            std::cout << "Invalid choice. Enter 1-6.\n";
        }
    }
}
