**System Requirements Specifications: Hospital Management Console
System**

------------------------------------------------------------------------

**1. Introduction**

**1.1 Purpose**

This document specifies the functional and non-functional requirements
for a **C++ Console-based Hospital Management System**. The system
provides a standalone interface for managing patient records and
appointments using a local file-based database.

------------------------------------------------------------------------

**1.2 System Scope**

The system is a simplified standalone utility focusing on
**receptionist-side operations** such as patient registration and
appointment scheduling.

The system:

- Does not include authentication or multiple roles

- Uses a local JSON file for persistence

- Focuses on simplicity, clarity, and maintainability

------------------------------------------------------------------------

**2. Functional Requirements**

**Actor: Receptionist**

------------------------------------------------------------------------

**2.1 System Initialization**

**FR-IN-01:** The system shall load all patient records and appointments
from the local JSON file at startup.

**FR-IN-02:** If the file does not exist, the system shall initialize
empty patient and appointment lists.

------------------------------------------------------------------------

**2.2 Main Menu**

**FR-MM-01:** Upon startup, the system shall present the following
options:

- Register Patient

- Schedule Appointment

- View Patient Records

- Update Patient Information

- Cancel Appointment

- Exit

------------------------------------------------------------------------

**2.3 Register Patient**

**FR-RP-01:** The user shall enter patient details (name, age).

**FR-RP-02:** The system shall assign a unique ID to the patient.

**FR-RP-03:** The system shall store the patient and persist it to the
JSON file.

------------------------------------------------------------------------

**2.4 Schedule Appointment**

**FR-SA-01:** The user shall select a patient by ID.

**FR-SA-02:** The user shall enter appointment details (date, time).

**FR-SA-03:** The system shall validate that the time slot is available.

**FR-SA-04:** The system shall store the appointment and persist it.

------------------------------------------------------------------------

**2.5 View Patient Records**

**FR-VP-01:** The system shall display all registered patients.

**FR-VP-02:** Each patient shall include:

- ID

- Name

- Age

**FR-VP-03:** The system may display associated appointments for each
patient.

------------------------------------------------------------------------

**2.6 Update Patient Information**

**FR-UP-01:** The user shall select a patient by ID.

**FR-UP-02:** The user shall enter updated information.

**FR-UP-03:** The system shall update the record and persist changes.

------------------------------------------------------------------------

**2.7 Cancel Appointment**

**FR-CA-01:** The user shall select an appointment by ID.

**FR-CA-02:** The system shall remove the appointment.

**FR-CA-03:** The system shall update the JSON file.

------------------------------------------------------------------------

**3. Non-Functional Requirements**

**3.1 Persistence & Reliability**

**NFR-REL-01:** All data must be stored in a local hospital.json file
using *nlohmann/json*.

**NFR-REL-02:** The system must ensure:

- No duplicate patient IDs

- No overlapping appointments

------------------------------------------------------------------------

**3.2 Usability**

**NFR-USA-01:** The system shall provide a simple and clear console
interface.

**NFR-USA-02:** The system shall validate all user inputs (invalid IDs,
incorrect formats).

------------------------------------------------------------------------

**3.3 Design Constraints**

**NFR-CON-01:** The codebase must be implemented in C++ and remain under
approximately **500 lines of code**.

**NFR-CON-02:** The system shall be divided into a maximum of **5
logical modules**.

**NFR-CON-03:** The system shall be console-based only.

**3.4 Technical Constraints**

**NFR-TECH-01:** The system shall use C++11 or later.

**NFR-TECH-02:** The system shall use *nlohmann/json* for persistence.

**NFR-TECH-03:** The system shall avoid advanced C++ features such as:

- Complex templates

- Metaprogramming

- Advanced inheritance

------------------------------------------------------------------------

**3.5 Testability**

**NFR-TST-01:** The business logic shall be independent of the console
interface.

**NFR-TST-02:** The logic layer shall be unit-testable without file or
UI dependencies.

------------------------------------------------------------------------

**4. System Specifications**

**4.1 Module Structure**

<table style="width:90%;">
<colgroup>
<col style="width: 23%" />
<col style="width: 66%" />
</colgroup>
<thead>
<tr>
<th><strong>Module Name</strong></th>
<th><strong>Responsibility</strong></th>
</tr>
</thead>
<tbody>
<tr>
<td><strong>HospitalConsole</strong></td>
<td>Handles UI loop, menu display, and user interaction</td>
</tr>
<tr>
<td><strong>HospitalService</strong></td>
<td>Manages business logic (patients, appointments, validation)</td>
</tr>
<tr>
<td><strong>HospitalRepository</strong></td>
<td>Handles JSON file operations</td>
</tr>
<tr>
<td><strong>Patient</strong></td>
<td>Data model representing a patient</td>
</tr>
<tr>
<td><strong>Appointment</strong></td>
<td>Data model representing an appointment</td>
</tr>
</tbody>
</table>

------------------------------------------------------------------------

**4.2 Data Model (JSON Schema)**

{\
"patients": \[\
{\
"id": 1,\
"name": "John Doe",\
"age": 30\
}\
\],\
"appointments": \[\
{\
"id": 1,\
"patientId": 1,\
"date": "2026-04-10",\
"time": "10:00"\
}\
\]\
}

------------------------------------------------------------------------

**4.3 Technical Standards**

- **Language:** C++11 or later

- **Libraries:**

  - *nlohmann/json* for persistence

- **Data Structures:**

  - std::vector for storing patients and appointments

  - std::string for names and time/date

------------------------------------------------------------------------

**4.4 Development Constraints for Code Minimalism and Simplicity**

- Favor simple and direct implementations

- Avoid unnecessary abstractions

- Maintain strict separation between:

  - Presentation layer

  - Logic layer

  - Data layer

- Keep total LOC within ~500 lines

- Avoid advanced features such as:

  - Complex design patterns

  - Heavy template usage

  - Over-engineering

The resulting system should be:

- Easy to understand

- Easy to maintain

- Highly testable
