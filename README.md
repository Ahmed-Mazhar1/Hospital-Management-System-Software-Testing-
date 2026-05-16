[README_5.md](https://github.com/user-attachments/files/27853881/README_5.md)
# 🏥 Hospital Management System — Software Testing Project

> **Language:** C++ (C++11 or later)  
> **Type:** Console-based standalone application  
> **Persistence:** Local JSON file (`hospital.json`) via *nlohmann/json*  
> **Test Plan Version:** 1.0 — May 4, 2026

---

## 📋 Table of Contents

- [About the Project](#about-the-project)
- [System Features](#system-features)
- [Module Structure](#module-structure)
- [Data Model](#data-model)
- [Non-Functional Requirements](#non-functional-requirements)
- [Testing Strategy](#testing-strategy)
- [Test Levels](#test-levels)
- [Risk Analysis](#risk-analysis)
- [Entry & Exit Criteria](#entry--exit-criteria)
- [Test Deliverables](#test-deliverables)
- [Repository Structure](#repository-structure)
- [How to Run](#how-to-run)
- [Team Members](#team-members)

---

## 📖 About the Project

This repository contains the **software testing project** for a C++ Console-based Hospital Management System (HMCS). The goal is to ensure the system meets all functional and non-functional requirements specified in the SRS and adheres to the architectural design depicted in the UML Class and Sequence diagrams, while identifying key defects.

The system focuses on **receptionist-side operations** and:
- Does not include authentication or multiple roles
- Uses a local JSON file for persistence
- Focuses on simplicity, clarity, and maintainability

---

## 🖥️ System Features

The system presents the following main menu options:

- **Register Patient** — Enter patient details (name, age); system assigns a unique ID and persists to JSON
- **Schedule Appointment** — Select a patient by ID, enter date and time; system validates slot availability and persists
- **View Patient Records** — Display all registered patients with ID, name, and age (with associated appointments)
- **Update Patient Information** — Select a patient by ID, enter updated info, persist changes
- **Cancel Appointment** — Select an appointment by ID, remove it, and update the JSON file
- **Exit**

On startup, the system loads all patient records and appointments from the local JSON file. If the file does not exist, it initializes empty lists.

---

## 🗂️ Module Structure

| Module | Responsibility |
|--------|---------------|
| **HospitalConsole** | Handles UI loop, menu display, and user interaction |
| **HospitalService** | Manages business logic (patients, appointments, validation) |
| **HospitalRepository** | Handles JSON file operations |
| **Patient** | Data model representing a patient |
| **Appointment** | Data model representing an appointment |

---

## 📐 Data Model

```json
{
  "patients": [
    {
      "id": 1,
      "name": "John Doe",
      "age": 30
    }
  ],
  "appointments": [
    {
      "id": 1,
      "patientId": 1,
      "date": "2026-04-10",
      "time": "10:00"
    }
  ]
}
```

**Data Structures used:**
- `std::vector` for storing patients and appointments
- `std::string` for names and time/date fields

---

## ⚙️ Non-Functional Requirements

**Persistence & Reliability**
- All data stored in `hospital.json` using *nlohmann/json*
- No duplicate patient IDs
- No overlapping appointments

**Usability**
- Simple and clear console interface
- All user inputs validated (invalid IDs, incorrect formats)

**Design Constraints**
- Implemented in C++; codebase stays under ~500 lines per module
- Maximum of 5 logical modules
- Console-based only; no complex templates, metaprogramming, or advanced inheritance

**Testability**
- Business logic is independent of the console interface
- Logic layer is unit-testable without file or UI dependencies

---

## 🧪 Testing Strategy

Testing covers both **static** and **dynamic** approaches:

- **Static Testing** — Technical review of the SRS and C++ source code to identify logic flaws or deviations from UML design before execution, and to verify design constraints are met
- **Dynamic Testing** — Execute the compiled program with specific test data to validate behavior against requirements FR-IN-01 through FR-CA-03

**Scope:**
- **Static:** Manual review of source code to verify adherence to design constraints (≤5 modules, ≤500 lines each)
- **Dynamic:** Entering invalid IDs, incorrect date formats, and testing system behavior
- **Unit Testing:** Targeting HospitalService, Patient, and Appointment modules in isolation
- **Integration Testing:** Interaction between C++ logic modules and the local JSON file
- **System Testing:** End-to-end execution of patient registration, appointment scheduling, and record management

---

## ✅ Test Levels

| Level | What | Technique |
|-------|------|-----------|
| **Unit Testing** | Input validation, business logic, and data integrity | White-Box Testing (Branch coverage) |
| **Integration Testing** | Interaction between modules and *nlohmann/json* — data flow from JSON to objects and back | Black-Box Testing (interface integrity) |
| **System Testing** | Full receptionist workflow: adding, removing, updating, and checking appointments from start to exit | End-to-End / Functional Testing (based on SRS, from the receptionist's perspective) |

**Pass/Fail Criteria:**
- ✅ **Pass** — System performs the function as described in the SRS and the JSON database reflects the change correctly
- ❌ **Fail** — Any deviation from the SRS, such as allowing duplicate IDs, failing to update the JSON file, or accepting wrong input formats (age, name, date, time)

---

## ⚠️ Risk Analysis

| Risk | Likelihood | Impact |
|------|-----------|--------|
| System fails to assign unique patient IDs | Medium | Critical |
| Failure to correctly link and display appointments with patient profiles | Low | Medium |
| Failure to persist data or corruption of the JSON file during save | Medium | Critical |
| Failure to permanently remove appointment records from the system and JSON file | High | High |
| Codebase exceeds the 500-line limit | High | Medium |
| Inaccurate validation of time slots, leading to overlapping appointments | Medium | Critical |

---

## 🚦 Entry & Exit Criteria

### Entry Criteria
Before testing begins, the following must be met:
- All 5 C++ modules must be coded and compile without errors
- Test Plan and Test Case Specifications must be documented and reviewed
- A valid `hospital.json` file with at least two sets of test data must be available
- A C++11 (or later) compiler and *nlohmann/json* library must be correctly configured
- Source code must have passed a basic walkthrough against the Sequence Diagram

### Exit Criteria
Testing is considered complete when:
- **80%** of functional requirements (FR-MM-01 through FR-CA-03) have been executed and passed
- All **Critical** and **High** severity defects are fixed and verified
- Every patient and appointment write-back to `hospital.json` is verified
- Final codebase is confirmed under the 500-line limit across the planned modules
- All local operations (retrieval and file writing) respond in **less than 1 second**
- Test Log and Defect Reports are completed and signed off

---

## 📄 Test Deliverables

- Test Plan
- Test Case Specifications
- Reviews
- Test Log / Execution Report
- Defect Reports

---

## 📁 Repository Structure

```
Hospital-Management-System-Software-Testing-/
│
├── ProjectCode/          # C++ source code of the Hospital Management System (SUT)
│
├── Diagrams/             # UML diagrams (Class and Sequence)
│
├── Docx/                 # Project documentation (test plans, test cases, reports)
│
└── README.md             # You are here
```

---

## ▶️ How to Run

### Prerequisites
- A C++ compiler supporting C++11 or later (e.g., **g++**, **MinGW**, **MSVC**)
- *nlohmann/json* library integrated into the source folder
- A pre-populated `hospital.json` file with at least two test patients with appointments

### Steps

```bash
# 1. Clone the repository
git clone https://github.com/Ahmed-Mazhar1/Hospital-Management-System-Software-Testing-.git

# 2. Navigate to the source code folder
cd Hospital-Management-System-Software-Testing-/ProjectCode

# 3. Compile the project
g++ -std=c++11 -o HospitalSystem main.cpp

# 4. Run the executable
./HospitalSystem
```

> On Windows, use `HospitalSystem.exe` instead of `./HospitalSystem`.

---

## 👥 Team Members

| Name |
|------|
| Ahmed Amr Ahmed Ismail Jabr |
| Abdelrahman Tarek Gamal Kilany Haggag |
| Ahmed Hossam Mohamed Ezz-Eldin Mohamed Mazhar |
| Martin Adel Hanna |
| Mohamed Amr Mohamed Salama Mohamed Eissa |
