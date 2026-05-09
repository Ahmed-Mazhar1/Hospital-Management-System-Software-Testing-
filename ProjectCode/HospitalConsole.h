#ifndef HOSPITAL_CONSOLE_H
#define HOSPITAL_CONSOLE_H

#include <string>

#include "HospitalService.h"

class HospitalConsole {
private:
    HospitalService& service;

    void showMenu() const;
    void registerPatient();
    void scheduleAppointment();
    void viewPatientRecords();
    void updatePatientInformation();
    void cancelAppointment();
    int readInt(const std::string& prompt) const;
    std::string readString(const std::string& prompt) const;

public:
    explicit HospitalConsole(HospitalService& service);

    void run();
};

#endif
