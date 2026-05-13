#include "pch.h"
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>

#include "HospitalService.h"
#include "HospitalRepository.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------
static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path.c_str(), std::ios::trunc);
    out << content;
}

static void removeFile(const std::string& path) {
    std::remove(path.c_str());
}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
class ServiceFixture : public ::testing::Test {
protected:
    std::string testFile;
    HospitalService* service;
    HospitalRepository* repo;

    void SetUp() override {
        testFile = "test_hospital_tmp.json";
        removeFile(testFile);
        service = new HospitalService();
        repo = new HospitalRepository(testFile);
    }

    void TearDown() override {
        delete service;
        delete repo;
        removeFile(testFile);
    }

    Patient addAlice() {
        return service->registerPatient("Alice", 30, *repo);
    }

    Appointment addAliceWithAppointment() {
        addAlice();
        return service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    }
};


// =============================================================================
// Suite: ServiceInit  (8 → 4)
// =============================================================================

// TC-INIT-001 – Missing file → starts empty (covers the "no file" branch)
TEST_F(ServiceFixture, Init_MissingFile_StartsEmpty) {
    service->initialize(*repo);
    EXPECT_TRUE(service->getPatients().empty());
    EXPECT_TRUE(service->getAppointmentsForPatient(1).empty());
}

// TC-INIT-002 – Valid file → patients and appointments loaded correctly
TEST_F(ServiceFixture, Init_ValidFile_LoadsData) {
    writeFile(testFile,
        "{"
        "  \"patients\":    [{\"id\":5,\"name\":\"Bob\",\"age\":40}],"
        "  \"appointments\":[{\"id\":3,\"patientId\":5,\"date\":\"2099-01-01\",\"time\":\"10:00\"}]"
        "}");

    service->initialize(*repo);

    std::vector<Patient> patients = service->getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ(5, patients[0].id);
    EXPECT_EQ("Bob", patients[0].name);
    EXPECT_EQ(40, patients[0].age);

    std::vector<Appointment> appts = service->getAppointmentsForPatient(5);
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ(3, appts[0].id);
    EXPECT_EQ("2099-01-01", appts[0].date);
    EXPECT_EQ("10:00", appts[0].time);
}

// TC-INIT-003 – Corrupted JSON → starts empty, no crash
TEST_F(ServiceFixture, Init_CorruptedFile_StartsEmpty) {
    writeFile(testFile, "{ this is not valid JSON {{{{");
    service->initialize(*repo);    // must not throw
    EXPECT_TRUE(service->getPatients().empty());
}

// TC-INIT-005 – nextPatientId picks up from max loaded id
TEST_F(ServiceFixture, Init_NextPatientIdContinuesFromMax) {
    writeFile(testFile,
        "{"
        "  \"patients\":    [{\"id\":10,\"name\":\"Eve\",\"age\":25}],"
        "  \"appointments\":[]"
        "}");
    service->initialize(*repo);
    Patient p = service->registerPatient("Frank", 28, *repo);
    EXPECT_EQ(11, p.id);
}

// TC-INIT-006 – nextAppointmentId picks up from max loaded appointment id
TEST_F(ServiceFixture, Init_NextAppointmentIdContinuesFromMax) {
    writeFile(testFile,
        "{"
        "  \"patients\":    [{\"id\":1,\"name\":\"Grace\",\"age\":22}],"
        "  \"appointments\":[{\"id\":7,\"patientId\":1,\"date\":\"2099-03-01\",\"time\":\"08:00\"}]"
        "}");
    service->initialize(*repo);
    Appointment a = service->scheduleAppointment(1, "2099-04-01", "09:00", *repo);
    EXPECT_EQ(8, a.id);
}


// =============================================================================
// Suite: RegisterPatient  (14 → 8)
// =============================================================================

// TC-REG-001 – Valid registration: returns correct id, name, age and persists
TEST_F(ServiceFixture, Register_Valid_ReturnsCorrectPatient) {
    Patient p = service->registerPatient("Alice", 30, *repo);
    EXPECT_EQ(1, p.id);
    EXPECT_EQ("Alice", p.name);
    EXPECT_EQ(30, p.age);

    // Also verify it appears in getPatients() — avoids a separate duplicate test
    std::vector<Patient> patients = service->getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice", patients[0].name);
}

// TC-REG-003 – Sequential registrations receive incrementing IDs
TEST_F(ServiceFixture, Register_MultiplePatients_IdsIncrement) {
    Patient p1 = service->registerPatient("Alice", 30, *repo);
    Patient p2 = service->registerPatient("Bob", 25, *repo);
    Patient p3 = service->registerPatient("Carol", 40, *repo);
    EXPECT_EQ(1, p1.id);
    EXPECT_EQ(2, p2.id);
    EXPECT_EQ(3, p3.id);
}

// TC-REG-N001 – Empty name throws runtime_error
TEST_F(ServiceFixture, Register_EmptyName_ThrowsException) {
    EXPECT_THROW(service->registerPatient("", 30, *repo), std::runtime_error);
}

// TC-REG-N002 – Exception message is exactly "Invalid patient information."
TEST_F(ServiceFixture, Register_EmptyName_CorrectExceptionMessage) {
    try {
        service->registerPatient("", 30, *repo);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ("Invalid patient information.", e.what());
    }
}

// TC-REG-N003 – Age 0 throws (covers age <= 0 branch; negative age hits same branch)
TEST_F(ServiceFixture, Register_AgeZero_ThrowsException) {
    EXPECT_THROW(service->registerPatient("Alice", 0, *repo), std::runtime_error);
}

// TC-REG-N005 – Age 130 (== boundary, not < 130) throws
TEST_F(ServiceFixture, Register_AgeBoundary130_ThrowsException) {
    EXPECT_THROW(service->registerPatient("Alice", 130, *repo), std::runtime_error);
}

// TC-REG-BVA-001 – Age 1 (minimum valid) accepted
TEST_F(ServiceFixture, Register_Age1_Accepted) {
    EXPECT_NO_THROW(service->registerPatient("Alice", 1, *repo));
}

// TC-REG-BVA-002 – Age 129 (maximum valid: < 130) accepted
TEST_F(ServiceFixture, Register_Age129_Accepted) {
    EXPECT_NO_THROW(service->registerPatient("Alice", 129, *repo));
}

// TC-REG-INT-001 – Failed registration does not add patient to list
TEST_F(ServiceFixture, Register_InvalidInput_DoesNotAddToList) {
    try { service->registerPatient("", 30, *repo); }
    catch (...) {}
    EXPECT_TRUE(service->getPatients().empty());
}


// =============================================================================
// Suite: ScheduleAppointment  (7 → 5)
// =============================================================================

// TC-APT-001 – Valid scheduling returns correct appointment data
TEST_F(ServiceFixture, Schedule_Valid_ReturnsCorrectAppointment) {
    addAlice();
    Appointment a = service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    EXPECT_EQ(1, a.id);
    EXPECT_EQ(1, a.patientId);
    EXPECT_EQ("2099-06-15", a.date);
    EXPECT_EQ("09:00", a.time);
}

// TC-APT-002 – Appointment IDs auto-increment across patients
TEST_F(ServiceFixture, Schedule_MultipleAppointments_IdsIncrement) {
    addAlice();
    service->registerPatient("Bob", 25, *repo);
    Appointment a1 = service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    Appointment a2 = service->scheduleAppointment(2, "2099-06-15", "10:00", *repo);
    EXPECT_EQ(1, a1.id);
    EXPECT_EQ(2, a2.id);
}

// TC-APT-N001 – Non-existent patient throws with correct message
TEST_F(ServiceFixture, Schedule_NonExistentPatient_Throws) {
    try {
        service->scheduleAppointment(999, "2099-06-15", "09:00", *repo);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ("Patient ID does not exist.", e.what());
    }
}

// TC-APT-N002 – Invalid date format throws with correct message
TEST_F(ServiceFixture, Schedule_InvalidDateFormat_Throws) {
    addAlice();
    try {
        service->scheduleAppointment(1, "15-06-2099", "09:00", *repo);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ("Invalid date or time format.", e.what());
    }
}

// TC-APT-N003 – Invalid time format throws with correct message
TEST_F(ServiceFixture, Schedule_InvalidTimeFormat_Throws) {
    addAlice();
    try {
        service->scheduleAppointment(1, "2099-06-15", "9:00", *repo);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ("Invalid date or time format.", e.what());
    }
}

// TC-APT-N004 – Same date+time slot throws with correct message (global conflict)
TEST_F(ServiceFixture, Schedule_OverlappingSlot_Throws) {
    addAlice();
    service->registerPatient("Bob", 25, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    try {
        service->scheduleAppointment(2, "2099-06-15", "09:00", *repo);
        FAIL() << "Expected std::runtime_error";
    }
    catch (const std::runtime_error& e) {
        EXPECT_STREQ("Appointment slot is already taken.", e.what());
    }
}

// TC-APT-005 – Same patient, different times: both accepted (no false conflict)
TEST_F(ServiceFixture, Schedule_SamePatientDifferentTimes_BothAccepted) {
    addAlice();
    EXPECT_NO_THROW(service->scheduleAppointment(1, "2099-06-15", "09:00", *repo));
    EXPECT_NO_THROW(service->scheduleAppointment(1, "2099-06-15", "10:00", *repo));
}


// =============================================================================
// Suite: GetPatients  (5 → 3)
// =============================================================================

// TC-VIEW-002 – getPatients() returns all registered patients
// (empty-list case is implicitly covered by Init_MissingFile)
TEST_F(ServiceFixture, GetPatients_AfterRegistrations_ReturnsAll) {
    service->registerPatient("Alice", 30, *repo);
    service->registerPatient("Bob", 25, *repo);
    EXPECT_EQ(2u, service->getPatients().size());
}

// TC-VIEW-003 – getAppointmentsForPatient() returns empty for patient with none
TEST_F(ServiceFixture, GetAppointments_PatientWithNone_ReturnsEmpty) {
    addAlice();
    EXPECT_TRUE(service->getAppointmentsForPatient(1).empty());
}

// TC-VIEW-004 – getAppointmentsForPatient() returns only the requested patient's records
TEST_F(ServiceFixture, GetAppointments_ReturnsOnlyForRequestedPatient) {
    addAlice();
    service->registerPatient("Bob", 25, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    service->scheduleAppointment(2, "2099-06-15", "10:00", *repo);

    std::vector<Appointment> aliceAppts = service->getAppointmentsForPatient(1);
    std::vector<Appointment> bobAppts = service->getAppointmentsForPatient(2);

    ASSERT_EQ(1u, aliceAppts.size());
    EXPECT_EQ(1, aliceAppts[0].patientId);

    ASSERT_EQ(1u, bobAppts.size());
    EXPECT_EQ(2, bobAppts[0].patientId);
}


// =============================================================================
// Suite: UpdatePatient  (9 → 6)
// =============================================================================

// TC-UPD-001 – Valid update returns true and modifies patient in-memory
TEST_F(ServiceFixture, Update_ValidData_ReturnsTrueAndUpdates) {
    addAlice();
    bool result = service->updatePatient(1, "Alice Updated", 35, *repo);
    ASSERT_TRUE(result);
    std::vector<Patient> patients = service->getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice Updated", patients[0].name);
    EXPECT_EQ(35, patients[0].age);
}

// TC-UPD-N001 – Non-existent patient ID returns false
TEST_F(ServiceFixture, Update_NonExistentId_ReturnsFalse) {
    EXPECT_FALSE(service->updatePatient(999, "Alice", 30, *repo));
}

// TC-UPD-N002 – Empty name returns false
TEST_F(ServiceFixture, Update_EmptyName_ReturnsFalse) {
    addAlice();
    EXPECT_FALSE(service->updatePatient(1, "", 30, *repo));
}

// TC-UPD-N003/N004 – Age 0 returns false (covers age <= 0 branch for update path)
TEST_F(ServiceFixture, Update_InvalidAge_ReturnsFalse) {
    addAlice();
    EXPECT_FALSE(service->updatePatient(1, "Alice", 0, *repo));  // zero
    EXPECT_FALSE(service->updatePatient(1, "Alice", -5, *repo));  // negative — same branch
    EXPECT_FALSE(service->updatePatient(1, "Alice", 130, *repo));  // >= 130 boundary
}

// TC-UPD-BVA-002 – Age 129 (max valid) succeeds; age 1 (min valid) covered by register BVA
TEST_F(ServiceFixture, Update_Age129_Succeeds) {
    addAlice();
    EXPECT_TRUE(service->updatePatient(1, "Alice", 129, *repo));
}

// TC-UPD-003 – Failed update does not change original data
TEST_F(ServiceFixture, Update_InvalidInput_OriginalDataUnchanged) {
    addAlice();
    service->updatePatient(1, "", 0, *repo);   // returns false
    std::vector<Patient> patients = service->getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice", patients[0].name);
    EXPECT_EQ(30, patients[0].age);
}

// TC-UPD-004 – Update targets only the correct patient; others unchanged
TEST_F(ServiceFixture, Update_OnlyTargetPatientChanged) {
    addAlice();
    service->registerPatient("Bob", 25, *repo);
    service->updatePatient(1, "Alice V2", 31, *repo);
    std::vector<Patient> patients = service->getPatients();
    EXPECT_EQ("Bob", patients[1].name);
    EXPECT_EQ(25, patients[1].age);
}


// =============================================================================
// Suite: CancelAppointment  (5 → 4)
// =============================================================================

// TC-CAN-001/002 – Valid cancel returns true AND removes from list (merged)
TEST_F(ServiceFixture, Cancel_ValidId_AppointmentRemovedFromList) {
    addAliceWithAppointment();
    EXPECT_TRUE(service->cancelAppointment(1, *repo));          // returns true
    EXPECT_TRUE(service->getAppointmentsForPatient(1).empty()); // removed
}

// TC-CAN-N001 – Non-existent appointment ID returns false
TEST_F(ServiceFixture, Cancel_NonExistentId_ReturnsFalse) {
    EXPECT_FALSE(service->cancelAppointment(999, *repo));
}

// TC-CAN-N002 – Double cancellation: second call returns false
TEST_F(ServiceFixture, Cancel_DoubleDeletion_SecondReturnsFalse) {
    addAliceWithAppointment();
    EXPECT_TRUE(service->cancelAppointment(1, *repo));
    EXPECT_FALSE(service->cancelAppointment(1, *repo));
}

// TC-CAN-003 – Cancelling one appointment leaves others for same patient intact
TEST_F(ServiceFixture, Cancel_OneAppointment_OthersSurvive) {
    addAlice();
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);  // id 1
    service->scheduleAppointment(1, "2099-06-15", "10:00", *repo);  // id 2
    service->cancelAppointment(1, *repo);
    std::vector<Appointment> appts = service->getAppointmentsForPatient(1);
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ(2, appts[0].id);
}

// TC-CAN-004 – Patient record intact after all its appointments are cancelled
TEST_F(ServiceFixture, Cancel_AllAppointments_PatientStillExists) {
    addAliceWithAppointment();
    service->cancelAppointment(1, *repo);
    std::vector<Patient> patients = service->getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice", patients[0].name);
}
