#include "HospitalRepository.h"

#include <fstream>

#include "json.hpp"

using nlohmann::json;

namespace {
json loadRoot(const std::string& filePath) {
    std::ifstream input(filePath.c_str());
    if (!input) {
        return json();
    }

    try {
        json root;
        input >> root;
        return root;
    } catch (...) {
        return json();
    }
}
}

HospitalRepository::HospitalRepository(const std::string& path) : filePath(path) {}

std::vector<Patient> HospitalRepository::loadPatients() const {
    std::vector<Patient> patients;
    json root = loadRoot(filePath);
    if (!root.is_object() || !root.contains("patients") || !root["patients"].is_array()) {
        return patients;
    }

    for (std::size_t i = 0; i < root["patients"].size(); ++i) {
        const json& item = root["patients"][i];
        if (item.contains("id") && item.contains("name") && item.contains("age")) {
            patients.push_back(Patient(item["id"].get<int>(), item["name"].get<std::string>(), item["age"].get<int>()));
        }
    }
    return patients;
}

std::vector<Appointment> HospitalRepository::loadAppointments() const {
    std::vector<Appointment> appointments;
    json root = loadRoot(filePath);
    if (!root.is_object() || !root.contains("appointments") || !root["appointments"].is_array()) {
        return appointments;
    }

    for (std::size_t i = 0; i < root["appointments"].size(); ++i) {
        const json& item = root["appointments"][i];
        if (item.contains("id") && item.contains("patientId") && item.contains("date") && item.contains("time")) {
            appointments.push_back(Appointment(
                item["id"].get<int>(),
                item["patientId"].get<int>(),
                item["date"].get<std::string>(),
                item["time"].get<std::string>()));
        }
    }
    return appointments;
}

void HospitalRepository::saveAll(const std::vector<Patient>& patients, const std::vector<Appointment>& appointments) const {
    json root;
    root["patients"] = json::array();
    root["appointments"] = json::array();

    for (std::size_t i = 0; i < patients.size(); ++i) {
        json item;
        item["id"] = patients[i].id;
        item["name"] = patients[i].name;
        item["age"] = patients[i].age;
        root["patients"].push_back(item);
    }

    for (std::size_t i = 0; i < appointments.size(); ++i) {
        json item;
        item["id"] = appointments[i].id;
        item["patientId"] = appointments[i].patientId;
        item["date"] = appointments[i].date;
        item["time"] = appointments[i].time;
        root["appointments"].push_back(item);
    }

    std::ofstream output(filePath.c_str(), std::ios::trunc);
    output << root.dump(2);
}

bool HospitalRepository::fileExists() const {
    std::ifstream input(filePath.c_str());
    return input.good();
}
