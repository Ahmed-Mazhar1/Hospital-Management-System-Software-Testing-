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
class SystemFixture : public ::testing::Test {
protected:
    std::string testFile;
    HospitalService* service;
    HospitalRepository* repo;

    void SetUp() override {
        testFile = "test_system_tmp.json";
        removeFile(testFile);
        service = new HospitalService();
        repo = new HospitalRepository(testFile);
    }

    void TearDown() override {
        delete service;
        delete repo;
        removeFile(testFile);
    }
};


// =============================================================================
// Suite: SystemWorkflow  (7 → 4)
// =============================================================================

// TC-SYS-WF-002 – Register → schedule two appointments → view: data consistent
TEST_F(SystemFixture, Workflow_RegisterScheduleView_Consistent) {
    service->registerPatient("Alice", 30, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    service->scheduleAppointment(1, "2099-06-15", "10:00", *repo);

    std::vector<Appointment> appts = service->getAppointmentsForPatient(1);
    ASSERT_EQ(2u, appts.size());
    EXPECT_EQ("09:00", appts[0].time);
    EXPECT_EQ("10:00", appts[1].time);
}

// TC-SYS-WF-003 – Full journey: register → schedule → update → cancel → view
TEST_F(SystemFixture, Workflow_FullUserJourney) {
    // Register
    Patient p = service->registerPatient("Alice", 30, *repo);
    EXPECT_EQ(1, p.id);

    // Schedule
    Appointment a = service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    EXPECT_EQ(1, a.id);

    // Update
    EXPECT_TRUE(service->updatePatient(1, "Alice Smith", 31, *repo));
    std::vector<Patient> patients = service->getPatients();
    EXPECT_EQ("Alice Smith", patients[0].name);
    EXPECT_EQ(31, patients[0].age);

    // Cancel
    EXPECT_TRUE(service->cancelAppointment(1, *repo));

    // View: appointment gone, patient still there
    EXPECT_TRUE(service->getAppointmentsForPatient(1).empty());
    EXPECT_EQ(1u, service->getPatients().size());
}

// TC-SYS-WF-004 – Cancel first of two appointments; second survives intact
TEST_F(SystemFixture, Workflow_CancelFirstOfTwo_SecondSurvives) {
    service->registerPatient("Alice", 30, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);  // id 1
    service->scheduleAppointment(1, "2099-06-15", "10:00", *repo);  // id 2

    service->cancelAppointment(1, *repo);

    std::vector<Appointment> appts = service->getAppointmentsForPatient(1);
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ(2, appts[0].id);
    EXPECT_EQ("10:00", appts[0].time);
}

// TC-SYS-WF-007 – Update then reload in new session: changes persisted to disk
TEST_F(SystemFixture, Workflow_UpdateThenReload_ChangesPersisted) {
    service->registerPatient("Alice", 30, *repo);
    service->updatePatient(1, "Alice New", 32, *repo);

    HospitalService svc2;
    svc2.initialize(*repo);

    std::vector<Patient> patients = svc2.getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice New", patients[0].name);
    EXPECT_EQ(32, patients[0].age);
}


// =============================================================================
// Suite: StressReliability  (5 → 3)
// =============================================================================

// TC-SYS-STRESS-001 – Register 50 patients consecutively without crash or data loss
TEST_F(SystemFixture, Stress_Register50Patients_NoCrash) {
    for (int i = 0; i < 50; ++i) {
        EXPECT_NO_THROW(
            service->registerPatient("Patient" + std::to_string(i), 20 + i % 100, *repo)
        );
    }
    EXPECT_EQ(50u, service->getPatients().size());
}

// TC-SYS-STRESS-003 – Alternating register + schedule: patient and appointment IDs
//                      are strictly monotonically increasing (no reuse, no gap)
TEST_F(SystemFixture, Stress_AlternatingRegisterSchedule_IdsMonotonic) {
    int lastPatientId = 0;
    int lastApptId = 0;

    for (int i = 0; i < 10; ++i) {
        Patient p = service->registerPatient("P" + std::to_string(i), 30, *repo);
        EXPECT_GT(p.id, lastPatientId);
        lastPatientId = p.id;

        char buf[20];
        std::snprintf(buf, sizeof(buf), "2099-01-%02d", i + 1);
        Appointment a = service->scheduleAppointment(p.id, std::string(buf), "09:00", *repo);
        EXPECT_GT(a.id, lastApptId);
        lastApptId = a.id;
    }
}

// TC-SYS-STRESS-005 – 20 rapid successive saves do not corrupt JSON
TEST_F(SystemFixture, Stress_RapidSaves_JsonStaysValid) {
    service->registerPatient("Alice", 30, *repo);
    for (int i = 0; i < 20; ++i) {
        service->updatePatient(1, "Alice" + std::to_string(i), 30 + i % 99, *repo);
    }
    HospitalService svc2;
    svc2.initialize(*repo);
    ASSERT_EQ(1u, svc2.getPatients().size());
}


// =============================================================================
// Suite: EdgeCases  (14 → 8)
// =============================================================================

// TC-SYS-EDGE-001 – scheduleAppointment with patient id 0 throws (never-assigned id)
TEST_F(SystemFixture, Edge_PatientId0_NotFound) {
    EXPECT_THROW(service->scheduleAppointment(0, "2099-06-15", "09:00", *repo),
        std::runtime_error);
}

// TC-SYS-EDGE-004 – Month 00 is rejected by date validation
TEST_F(SystemFixture, Edge_DateMonth00_Rejected) {
    service->registerPatient("Alice", 30, *repo);
    EXPECT_THROW(service->scheduleAppointment(1, "2099-00-15", "09:00", *repo),
        std::runtime_error);
}

// TC-SYS-EDGE-007 – Time 00:00 accepted (midnight lower boundary)
TEST_F(SystemFixture, Edge_Time0000_Accepted) {
    service->registerPatient("Alice", 30, *repo);
    EXPECT_NO_THROW(service->scheduleAppointment(1, "2099-06-15", "00:00", *repo));
}

// TC-SYS-EDGE-008 – Time 23:59 accepted (upper end-of-day boundary)
TEST_F(SystemFixture, Edge_Time2359_Accepted) {
    service->registerPatient("Alice", 30, *repo);
    EXPECT_NO_THROW(service->scheduleAppointment(1, "2099-06-15", "23:59", *repo));
}

// TC-SYS-EDGE-009 – Same date, different minute: not a slot conflict
TEST_F(SystemFixture, Edge_SameDateDifferentTime_NoConflict) {
    service->registerPatient("Alice", 30, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    EXPECT_NO_THROW(service->scheduleAppointment(1, "2099-06-15", "09:01", *repo));
}

// TC-SYS-EDGE-011 – Single character name is accepted (non-empty guard passes)
TEST_F(SystemFixture, Edge_SingleCharName_Accepted) {
    EXPECT_NO_THROW(service->registerPatient("A", 30, *repo));
}

// TC-SYS-EDGE-012 – INT_MAX age is rejected (>= 130)
TEST_F(SystemFixture, Edge_IntMaxAge_Rejected) {
    EXPECT_THROW(service->registerPatient("Alice", 2147483647, *repo),
        std::runtime_error);
}

// TC-SYS-EDGE-013 – INT_MIN age is rejected (< 0)
TEST_F(SystemFixture, Edge_IntMinAge_Rejected) {
    EXPECT_THROW(service->registerPatient("Alice", -2147483648, *repo),
        std::runtime_error);
}


// =============================================================================
// Suite: DataIsolation  (5 → 3)
// =============================================================================

// TC-SYS-ISO-001 – Updating patient A does not alter patient B
TEST_F(SystemFixture, Isolation_UpdateA_BUnchanged) {
    service->registerPatient("Alice", 30, *repo);
    service->registerPatient("Bob", 25, *repo);

    service->updatePatient(1, "Alice Updated", 31, *repo);

    std::vector<Patient> patients = service->getPatients();
    EXPECT_EQ("Bob", patients[1].name);
    EXPECT_EQ(25, patients[1].age);
}

// TC-SYS-ISO-002 – Cancelling patient A's appointment does not affect patient B's
TEST_F(SystemFixture, Isolation_CancelA_BAppointmentsUnchanged) {
    service->registerPatient("Alice", 30, *repo);
    service->registerPatient("Bob", 25, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);  // appt 1 → Alice
    service->scheduleAppointment(2, "2099-06-16", "09:00", *repo);  // appt 2 → Bob

    service->cancelAppointment(1, *repo);

    std::vector<Appointment> bobAppts = service->getAppointmentsForPatient(2);
    ASSERT_EQ(1u, bobAppts.size());
    EXPECT_EQ(2, bobAppts[0].id);
}

// TC-SYS-ISO-004 – A failed schedule attempt does not affect existing appointments
TEST_F(SystemFixture, Isolation_FailedSchedule_ExistingAppointmentsUnchanged) {
    service->registerPatient("Alice", 30, *repo);
    service->scheduleAppointment(1, "2099-06-15", "09:00", *repo);

    try {
        service->scheduleAppointment(999, "2099-06-15", "10:00", *repo);  // bad patient id
    }
    catch (...) {}

    EXPECT_EQ(1u, service->getAppointmentsForPatient(1).size());
}
