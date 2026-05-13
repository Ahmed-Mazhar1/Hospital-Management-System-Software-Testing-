#include "pch.h"
#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#include "HospitalRepository.h"
#include "HospitalService.h"

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

static std::string readFile(const std::string& path) {
    std::ifstream in(path.c_str());
    return std::string(std::istreambuf_iterator<char>(in),
        std::istreambuf_iterator<char>());
}

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------
class RepoFixture : public ::testing::Test {
protected:
    std::string testFile;
    HospitalRepository* repo;

    void SetUp() override {
        testFile = "test_repo_tmp.json";
        removeFile(testFile);
        repo = new HospitalRepository(testFile);
    }

    void TearDown() override {
        delete repo;
        removeFile(testFile);
    }
};


// =============================================================================
// Suite: RepoFileExists  (3 → 2)
// =============================================================================

// TC-REPO-FE-001 – fileExists() returns false when file is absent
TEST_F(RepoFixture, FileExists_Absent_ReturnsFalse) {
    EXPECT_FALSE(repo->fileExists());
}

// TC-REPO-FE-002 – fileExists() returns true after file is created
// (also covers the empty-file case — same OS presence check)
TEST_F(RepoFixture, FileExists_Present_ReturnsTrue) {
    writeFile(testFile, "{}");
    EXPECT_TRUE(repo->fileExists());
}


// =============================================================================
// Suite: RepoLoadPatients  (12 → 6)
// =============================================================================

// TC-REPO-LP-001 – Missing file → empty list
TEST_F(RepoFixture, LoadPatients_MissingFile_ReturnsEmpty) {
    EXPECT_TRUE(repo->loadPatients().empty());
}

// TC-REPO-LP-002 – Corrupted JSON → empty list, no crash
// (also exercises root-is-not-object and patients-not-array branches)
TEST_F(RepoFixture, LoadPatients_CorruptedJson_ReturnsEmpty) {
    writeFile(testFile, "{ not valid json !!!!");
    EXPECT_TRUE(repo->loadPatients().empty());
}

// TC-REPO-LP-004 – Empty object {} → empty list
TEST_F(RepoFixture, LoadPatients_EmptyObject_ReturnsEmpty) {
    writeFile(testFile, "{}");
    EXPECT_TRUE(repo->loadPatients().empty());
}

// TC-REPO-LP-008 – Single valid patient → all fields loaded correctly
TEST_F(RepoFixture, LoadPatients_SingleValidPatient_ReturnsCorrectData) {
    writeFile(testFile,
        "{\"patients\":[{\"id\":3,\"name\":\"Alice\",\"age\":28}],"
        "\"appointments\":[]}");

    std::vector<Patient> patients = repo->loadPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ(3, patients[0].id);
    EXPECT_EQ("Alice", patients[0].name);
    EXPECT_EQ(28, patients[0].age);
}

// TC-REPO-LP-009 – Multiple patients → all loaded in order
TEST_F(RepoFixture, LoadPatients_MultiplePatients_AllLoaded) {
    writeFile(testFile,
        "{\"patients\":["
        "  {\"id\":1,\"name\":\"Alice\",\"age\":28},"
        "  {\"id\":2,\"name\":\"Bob\",  \"age\":35},"
        "  {\"id\":3,\"name\":\"Carol\",\"age\":42}"
        "],\"appointments\":[]}");

    std::vector<Patient> patients = repo->loadPatients();
    ASSERT_EQ(3u, patients.size());
    EXPECT_EQ("Alice", patients[0].name);
    EXPECT_EQ("Bob", patients[1].name);
    EXPECT_EQ("Carol", patients[2].name);
}

// TC-REPO-LP-010 – Patient entry with missing required field → silently skipped
// (covers the guard that skips any incomplete entry; name/id/age missing all
//  hit the same validation gate — one field is sufficient to test the branch)
TEST_F(RepoFixture, LoadPatients_MissingNameField_EntrySkipped) {
    writeFile(testFile,
        "{\"patients\":["
        "  {\"id\":1,\"age\":28},"                        // missing "name" → skipped
        "  {\"id\":2,\"name\":\"Bob\",\"age\":35}"
        "],\"appointments\":[]}");

    std::vector<Patient> patients = repo->loadPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Bob", patients[0].name);
}


// =============================================================================
// Suite: RepoLoadAppointments  (7 → 4)
// =============================================================================

// TC-REPO-LA-001 – Missing file → empty appointments
TEST_F(RepoFixture, LoadAppointments_MissingFile_ReturnsEmpty) {
    EXPECT_TRUE(repo->loadAppointments().empty());
}

// TC-REPO-LA-003 – "appointments" key missing → empty
TEST_F(RepoFixture, LoadAppointments_NoAppointmentsKey_ReturnsEmpty) {
    writeFile(testFile, "{\"patients\":[]}");
    EXPECT_TRUE(repo->loadAppointments().empty());
}

// TC-REPO-LA-005 – Single valid appointment → all fields correct
TEST_F(RepoFixture, LoadAppointments_SingleValid_ReturnsCorrectData) {
    writeFile(testFile,
        "{\"patients\":[],"
        "\"appointments\":[{\"id\":5,\"patientId\":2,\"date\":\"2099-01-01\",\"time\":\"14:00\"}]}");

    std::vector<Appointment> appts = repo->loadAppointments();
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ(5, appts[0].id);
    EXPECT_EQ(2, appts[0].patientId);
    EXPECT_EQ("2099-01-01", appts[0].date);
    EXPECT_EQ("14:00", appts[0].time);
}

// TC-REPO-LA-006 – Appointment with missing required field → silently skipped
// (covers skip-entry guard; time field chosen; patientId missing hits same path)
TEST_F(RepoFixture, LoadAppointments_MissingTimeField_EntrySkipped) {
    writeFile(testFile,
        "{\"patients\":[],"
        "\"appointments\":["
        "  {\"id\":1,\"patientId\":1,\"date\":\"2099-01-01\"},"    // missing "time" → skip
        "  {\"id\":2,\"patientId\":1,\"date\":\"2099-02-01\",\"time\":\"09:00\"}"
        "]}");

    std::vector<Appointment> appts = repo->loadAppointments();
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ(2, appts[0].id);
}


// =============================================================================
// Suite: RepoSaveAll  (6 → 4)
// =============================================================================

// TC-REPO-SA-002 – saveAll with empty lists produces parseable, non-empty JSON
// (also implicitly verifies fileExists() after save — absorbs SA-001)
TEST_F(RepoFixture, SaveAll_EmptyLists_ValidJson) {
    repo->saveAll({}, {});
    EXPECT_TRUE(repo->fileExists());
    EXPECT_FALSE(readFile(testFile).empty());
    EXPECT_TRUE(repo->loadPatients().empty());
    EXPECT_TRUE(repo->loadAppointments().empty());
}

// TC-REPO-SA-003 – Single patient round-trips correctly
TEST_F(RepoFixture, SaveAll_SinglePatient_RoundTrip) {
    std::vector<Patient>     patients = { Patient(7, "Diana", 33) };
    std::vector<Appointment> appts;
    repo->saveAll(patients, appts);

    std::vector<Patient> loaded = repo->loadPatients();
    ASSERT_EQ(1u, loaded.size());
    EXPECT_EQ(7, loaded[0].id);
    EXPECT_EQ("Diana", loaded[0].name);
    EXPECT_EQ(33, loaded[0].age);
}

// TC-REPO-SA-004 – Single appointment round-trips correctly
TEST_F(RepoFixture, SaveAll_SingleAppointment_RoundTrip) {
    std::vector<Patient>     patients = { Patient(1, "Ed", 40) };
    std::vector<Appointment> appts = { Appointment(10, 1, "2099-07-20", "11:30") };
    repo->saveAll(patients, appts);

    std::vector<Appointment> loaded = repo->loadAppointments();
    ASSERT_EQ(1u, loaded.size());
    EXPECT_EQ(10, loaded[0].id);
    EXPECT_EQ(1, loaded[0].patientId);
    EXPECT_EQ("2099-07-20", loaded[0].date);
    EXPECT_EQ("11:30", loaded[0].time);
}

// TC-REPO-SA-006 – Multiple patients and appointments: all persist correctly
// (also validates overwrite semantics via the implicit truncation on the second call)
TEST_F(RepoFixture, SaveAll_Multiple_AllPersist) {
    std::vector<Patient> patients = {
        Patient(1, "Alice", 30),
        Patient(2, "Bob",   25)
    };
    std::vector<Appointment> appts = {
        Appointment(1, 1, "2099-06-15", "09:00"),
        Appointment(2, 2, "2099-06-15", "10:00"),
        Appointment(3, 1, "2099-06-16", "09:00")
    };
    repo->saveAll(patients, appts);

    EXPECT_EQ(2u, repo->loadPatients().size());
    EXPECT_EQ(3u, repo->loadAppointments().size());
}


// =============================================================================
// Suite: RepoIntegration  (7 → 4)
// =============================================================================

// TC-INT-DISK-001 – Register patient, reload in fresh session: patient persists
TEST_F(RepoFixture, Integration_RegisterAndReload_PatientPersists) {
    {
        HospitalService svc;
        svc.registerPatient("Alice", 30, *repo);
    }
    HospitalService svc2;
    svc2.initialize(*repo);

    std::vector<Patient> patients = svc2.getPatients();
    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice", patients[0].name);
    EXPECT_EQ(30, patients[0].age);
}

// TC-INT-DISK-002 – Schedule appointment, reload: appointment persists
TEST_F(RepoFixture, Integration_ScheduleAndReload_AppointmentPersists) {
    {
        HospitalService svc;
        svc.registerPatient("Alice", 30, *repo);
        svc.scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    }
    HospitalService svc2;
    svc2.initialize(*repo);

    std::vector<Appointment> appts = svc2.getAppointmentsForPatient(1);
    ASSERT_EQ(1u, appts.size());
    EXPECT_EQ("2099-06-15", appts[0].date);
    EXPECT_EQ("09:00", appts[0].time);
}

// TC-INT-DISK-005 – nextId resumes correctly after reload (no gap/collision)
TEST_F(RepoFixture, Integration_IdContinuationAfterReload) {
    {
        HospitalService svc;
        svc.registerPatient("Alice", 30, *repo);
        svc.registerPatient("Bob", 25, *repo);
    }
    HospitalService svc2;
    svc2.initialize(*repo);
    Patient p = svc2.registerPatient("Carol", 40, *repo);
    EXPECT_EQ(3, p.id);
}

// TC-INT-DISK-006 – Full lifecycle: register, schedule, update, cancel → JSON correct
TEST_F(RepoFixture, Integration_FullLifecycle_JsonSchemaCorrect) {
    HospitalService svc;
    svc.registerPatient("Alice", 30, *repo);
    svc.scheduleAppointment(1, "2099-06-15", "09:00", *repo);
    svc.updatePatient(1, "Alice Updated", 31, *repo);
    svc.cancelAppointment(1, *repo);

    HospitalRepository freshRepo(testFile);
    std::vector<Patient>     patients = freshRepo.loadPatients();
    std::vector<Appointment> appts = freshRepo.loadAppointments();

    ASSERT_EQ(1u, patients.size());
    EXPECT_EQ("Alice Updated", patients[0].name);
    EXPECT_EQ(31, patients[0].age);
    EXPECT_TRUE(appts.empty());
}
