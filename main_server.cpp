#include "crow.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>

using json = nlohmann::json;

// =========================
// CSV parsing helpers
// =========================
// The data files may have been saved with Windows line endings (\r\n).
// std::getline only strips the \n, so without this the last column of every
// line (passwords, statuses, dates...) would silently keep a trailing '\r' -
// breaking exact string comparisons like login password checks. This also
// makes "blank" trailing lines in the .txt files register as real
// (non-empty) records, since a line containing only '\r' is technically not
// an empty string.
static inline std::string cleanField(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
    return s;
}
static inline bool isBlankLine(const std::string& s) {
    return s.find_first_not_of(" \t\r\n") == std::string::npos;
}
// Prevents user-typed commas/newlines from corrupting the CSV column layout
// (e.g. a doctor typing "Paracetamol, twice daily" as medication would
// otherwise shift every field after it, including labTestOrder/followUpDate).
static inline std::string sanitizeCsvField(std::string s) {
    for (auto& c : s) {
        if (c == ',') c = ';';
        if (c == '\n' || c == '\r') c = ' ';
    }
    return s;
}

// Fixed consultation fee (INR) and fixed daily slots, matching the CLI prototype.
static const int CONSULTATION_FEE = 500;
static const int SLOT_CAPACITY = 3;
static const std::vector<std::string> DAILY_SLOTS = {
    "09:00 AM - 10:00 AM",
    "11:00 AM - 12:00 PM",
    "02:00 PM - 03:00 PM"
};

// =========================
// Utility: Load Patients
// =========================
json loadPatients() {
    std::ifstream infile("patients.txt");
    json patients = json::array();
    std::string line;

    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string id, name, age, ailment, phone;
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, age, ',');
        std::getline(ss, ailment, ',');
        std::getline(ss, phone, ',');
        if (!id.empty()) {
            patients.push_back({
                {"id", id},
                {"name", name},
                {"age", age},
                {"ailment", ailment},
                {"phone", phone}
            });
        }
    }
    return patients;
}

// =========================
// Utility: Load Doctors (no passwords)
// =========================
json loadDoctors() {
    std::ifstream infile("doctors.txt");
    json doctors = json::array();
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string id, name, spec, pass;
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, spec, ',');
        std::getline(ss, pass, ',');
        if (!id.empty()) {
            doctors.push_back({{"id", id}, {"name", name}, {"specialization", spec}});
        }
    }
    return doctors;
}

// Returns the doctor's name/specialization if found.
bool findDoctor(const std::string& doctorId, std::string& name, std::string& spec) {
    std::ifstream infile("doctors.txt");
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string id, n, s, pass;
        std::getline(ss, id, ',');
        std::getline(ss, n, ',');
        std::getline(ss, s, ',');
        std::getline(ss, pass, ',');
        if (id == doctorId) { name = n; spec = s; return true; }
    }
    return false;
}

// How many appointments already occupy this doctor/slot combination.
int getSlotBookingCount(const std::string& doctorId, const std::string& slot) {
    std::ifstream infile("appointments.txt");
    int count = 0;
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string appId, patId, docId, timeSlot;
        std::getline(ss, appId, ',');
        std::getline(ss, patId, ',');
        std::getline(ss, docId, ',');
        std::getline(ss, timeSlot, ',');
        if (!docId.empty() && docId == doctorId && timeSlot == slot) count++;
    }
    return count;
}

// Finds the next unused numeric id in a text file's first column, starting from a floor value.
int getNextId(const std::string& filename, int floorValue) {
    std::ifstream infile(filename);
    int maxId = floorValue - 1;
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string idStr;
        std::getline(ss, idStr, ',');
        if (!idStr.empty()) {
            try {
                int id = std::stoi(idStr);
                if (id > maxId) maxId = id;
            } catch (...) {}
        }
    }
    return maxId + 1;
}

// Looks up a patient's age (as int) by id. Returns -1 if not found.
int getPatientAge(const std::string& patientId) {
    std::ifstream infile("patients.txt");
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string id, name, age;
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        std::getline(ss, age, ',');
        if (id == patientId) {
            try { return std::stoi(age); } catch (...) { return -1; }
        }
    }
    return -1;
}

std::string getPatientName(const std::string& patientId) {
    std::ifstream infile("patients.txt");
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string id, name;
        std::getline(ss, id, ',');
        std::getline(ss, name, ',');
        if (id == patientId) return name;
    }
    return "Unknown";
}

// True once the doctor has already written a prescription for this patient
// (i.e. the patient has been seen/treated and should drop off the active queue).
bool hasBeenTreated(const std::string& patientId, const std::string& doctorId) {
    std::ifstream infile("prescriptions.txt");
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string patId, docId;
        std::getline(ss, patId, ',');
        std::getline(ss, docId, ',');
        if (patId == patientId && docId == doctorId) return true;
    }
    return false;
}

// True once a lab report already exists on file for this patient
// (i.e. the ordered test has already been completed/reported).
bool hasLabReport(const std::string& patientId) {
    std::ifstream infile("lab_reports.txt");
    std::string line;
    while (std::getline(infile, line)) {
        line = cleanField(line);
        if (isBlankLine(line)) continue;
        std::stringstream ss(line);
        std::string patId;
        std::getline(ss, patId, ',');
        if (patId == patientId) return true;
    }
    return false;
}

int main() {
    crow::SimpleApp app;

    // =========================
    // Serve frontend files
    // =========================
    CROW_ROUTE(app, "/")([](){
        std::ifstream file("frontend/index.html");
        if (!file.is_open()) return crow::response(404, "index.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/style.css")([](){
        std::ifstream file("frontend/style.css");
        if (!file.is_open()) return crow::response(404, "style.css not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/css; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/app.js")([](){
        std::ifstream file("frontend/app.js");
        if (!file.is_open()) return crow::response(404, "app.js not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "application/javascript; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/patient.html")([](){
        std::ifstream file("frontend/patient.html");
        if (!file.is_open()) return crow::response(404, "patient.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/doctor.html")([](){
        std::ifstream file("frontend/doctor.html");
        if (!file.is_open()) return crow::response(404, "doctor.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    CROW_ROUTE(app, "/lab.html")([](){
        std::ifstream file("frontend/lab.html");
        if (!file.is_open()) return crow::response(404, "lab.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/html; charset=utf-8");
        return res;
    });

    // =========================
    // Doctor Login
    // =========================
    CROW_ROUTE(app, "/api/doctor/login").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::string doctorId = body["doctorId"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            std::ifstream infile("doctors.txt");
            std::string line;
            while (std::getline(infile, line)) {
                line = cleanField(line);
                if (isBlankLine(line)) continue;
                std::stringstream ss(line);
                std::string id, name, spec, pass;
                std::getline(ss, id, ',');
                std::getline(ss, name, ',');
                std::getline(ss, spec, ',');
                std::getline(ss, pass, ',');
                if (id == doctorId && pass == password) {
                    json res = {{"success", true}, {"name", name}, {"specialization", spec}};
                    return crow::response(200, res.dump());
                }
            }
            json res = {{"success", false}, {"message", "Invalid doctor ID or password"}};
            return crow::response(401, res.dump());
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Lab Login
    // =========================
    CROW_ROUTE(app, "/api/lab/login").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::string labId = body["labId"].get<std::string>();
            std::string password = body["password"].get<std::string>();

            std::ifstream infile("lab_staff.txt");
            std::string line;
            while (std::getline(infile, line)) {
                line = cleanField(line);
                if (isBlankLine(line)) continue;
                std::stringstream ss(line);
                std::string id, name, pass;
                std::getline(ss, id, ',');
                std::getline(ss, name, ',');
                std::getline(ss, pass, ',');
                if (id == labId && pass == password) {
                    json res = {{"success", true}, {"name", name}};
                    return crow::response(200, res.dump());
                }
            }
            json res = {{"success", false}, {"message", "Invalid lab staff ID or password"}};
            return crow::response(401, res.dump());
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Patients
    // =========================
    CROW_ROUTE(app, "/api/patients")
    ([](){ return crow::response{loadPatients().dump()}; });

    CROW_ROUTE(app, "/api/patients/add").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::ofstream file("patients.txt", std::ios::app);
            if (!file.is_open()) return crow::response(500, "Unable to open patients.txt");
            file << sanitizeCsvField(body["id"].get<std::string>()) << ","
                 << sanitizeCsvField(body["name"].get<std::string>()) << ","
                 << sanitizeCsvField(body["age"].get<std::string>()) << ","
                 << sanitizeCsvField(body["ailment"].get<std::string>()) << ","
                 << sanitizeCsvField(body["phone"].get<std::string>()) << "\n";
            return crow::response(200, "Patient added successfully!");
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Appointments
    // =========================
    CROW_ROUTE(app, "/api/appointments")
    ([](){
        std::ifstream infile("appointments.txt");
        json appointments = json::array();
        std::string line;
        while (std::getline(infile, line)) {
            line = cleanField(line);
            if (isBlankLine(line)) continue;
            std::stringstream ss(line);
            std::string appId, patId, docId, slot, paid, status;
            std::getline(ss, appId, ',');
            std::getline(ss, patId, ',');
            std::getline(ss, docId, ',');
            std::getline(ss, slot, ',');
            std::getline(ss, paid, ',');
            std::getline(ss, status, ',');
            if (!appId.empty()) {
                appointments.push_back({
                    {"appointmentId", appId},
                    {"patientId", patId},
                    {"doctorId", docId},
                    {"timeSlot", slot},
                    {"feePaid", paid},
                    {"status", status}
                });
            }
        }
        return crow::response{appointments.dump()};
    });

    // =========================
    // Doctors list (for patient booking - specialists list)
    // =========================
    CROW_ROUTE(app, "/api/doctors")
    ([](){ return crow::response{loadDoctors().dump()}; });

    // =========================
    // Slot availability for a doctor (max 3 patients/slot)
    // =========================
    CROW_ROUTE(app, "/api/slots")
    ([](const crow::request& req){
        auto doctorId = req.url_params.get("doctorId");
        if (!doctorId) return crow::response(400, "doctorId query param required");
        std::string docId(doctorId);

        std::string name, spec;
        if (!findDoctor(docId, name, spec)) return crow::response(404, "Doctor not found");

        json slots = json::array();
        for (const auto& slot : DAILY_SLOTS) {
            int booked = getSlotBookingCount(docId, slot);
            slots.push_back({
                {"slot", slot},
                {"booked", booked},
                {"capacity", SLOT_CAPACITY},
                {"available", booked < SLOT_CAPACITY}
            });
        }
        json res = {{"doctorId", docId}, {"doctorName", name}, {"specialization", spec}, {"slots", slots}};
        return crow::response(200, res.dump());
    });

    // =========================
    // Book Appointment: registers the patient, checks slot capacity,
    // simulates payment, applies senior-citizen priority, and returns a receipt.
    // =========================
    CROW_ROUTE(app, "/api/appointments/book").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::string name = body["name"].get<std::string>();
            int age = std::stoi(body["age"].get<std::string>());
            std::string ailment = body["ailment"].get<std::string>();
            std::string phone = body["phone"].get<std::string>();
            std::string doctorId = body["doctorId"].get<std::string>();
            std::string timeSlot = body["timeSlot"].get<std::string>();
            bool payNow = body.value("payNow", true);

            std::string doctorName, spec;
            if (!findDoctor(doctorId, doctorName, spec)) {
                json res = {{"success", false}, {"message", "Invalid doctor ID"}};
                return crow::response(404, res.dump());
            }

            bool validSlot = std::find(DAILY_SLOTS.begin(), DAILY_SLOTS.end(), timeSlot) != DAILY_SLOTS.end();
            if (!validSlot) {
                json res = {{"success", false}, {"message", "Invalid time slot"}};
                return crow::response(400, res.dump());
            }

            if (getSlotBookingCount(doctorId, timeSlot) >= SLOT_CAPACITY) {
                json res = {{"success", false}, {"message", "That slot is fully booked (max " + std::to_string(SLOT_CAPACITY) + " patients). Please choose another slot."}};
                return crow::response(409, res.dump());
            }

            int patientId = getNextId("patients.txt", 1001);
            int appointmentId = getNextId("appointments.txt", 20001);

            std::string feePaidStr = payNow ? "Paid" : "Unpaid";
            std::string status;
            if (!payNow) {
                status = "Pending";
            } else if (age > 60) {
                status = "Confirmed (Senior Citizen Priority)";
            } else {
                status = "Confirmed";
            }

            std::ofstream patFile("patients.txt", std::ios::app);
            if (!patFile.is_open()) return crow::response(500, "Unable to open patients.txt");
            patFile << patientId << "," << sanitizeCsvField(name) << "," << age << "," << sanitizeCsvField(ailment) << "," << sanitizeCsvField(phone) << "\n";
            patFile.close();

            std::ofstream appFile("appointments.txt", std::ios::app);
            if (!appFile.is_open()) return crow::response(500, "Unable to open appointments.txt");
            appFile << appointmentId << "," << patientId << "," << doctorId << "," << timeSlot << "," << feePaidStr << "," << status << "\n";
            appFile.close();

            json res = {
                {"success", true},
                {"appointmentId", appointmentId},
                {"patientId", patientId},
                {"patientName", name},
                {"doctorId", doctorId},
                {"doctorName", doctorName},
                {"specialization", spec},
                {"timeSlot", timeSlot},
                {"fee", CONSULTATION_FEE},
                {"feePaid", feePaidStr},
                {"status", status}
            };
            return crow::response(200, res.dump());
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Doctor Queue: senior-citizen priority sorted appointment list
    // =========================
    CROW_ROUTE(app, "/api/doctor/queue")
    ([](const crow::request& req){
        auto doctorId = req.url_params.get("doctorId");
        if (!doctorId) return crow::response(400, "doctorId query param required");
        std::string docId(doctorId);

        std::ifstream infile("appointments.txt");
        json priorityQueue = json::array();
        json regularQueue = json::array();
        json treated = json::array();
        std::string line;
        while (std::getline(infile, line)) {
            line = cleanField(line);
            if (isBlankLine(line)) continue;
            std::stringstream ss(line);
            std::string appId, patId, docIdField, slot, paid, status;
            std::getline(ss, appId, ',');
            std::getline(ss, patId, ',');
            std::getline(ss, docIdField, ',');
            std::getline(ss, slot, ',');
            std::getline(ss, paid, ',');
            std::getline(ss, status, ',');
            if (docIdField != docId) continue;

            bool isSenior = status.find("Senior Citizen Priority") != std::string::npos;
            json entry = {
                {"appointmentId", appId},
                {"patientId", patId},
                {"patientName", getPatientName(patId)},
                {"age", getPatientAge(patId)},
                {"timeSlot", slot},
                {"feePaid", paid},
                {"status", status},
                {"priority", isSenior}
            };

            if (hasBeenTreated(patId, docId)) {
                // Already prescribed/seen - move out of the active queue.
                treated.push_back(entry);
                continue;
            }

            if (isSenior) priorityQueue.push_back(entry);
            else regularQueue.push_back(entry);
        }

        json queue = json::array();
        for (auto& e : priorityQueue) queue.push_back(e);
        for (auto& e : regularQueue) queue.push_back(e);

        json res = {{"doctorId", docId}, {"queue", queue}, {"treated", treated}};
        return crow::response(200, res.dump());
    });

    // =========================
    // Doctor Analytics & Revenue Dashboard
    // =========================
    CROW_ROUTE(app, "/api/doctor/analytics")
    ([](const crow::request& req){
        auto doctorId = req.url_params.get("doctorId");
        if (!doctorId) return crow::response(400, "doctorId query param required");
        std::string docId(doctorId);

        std::string doctorName, spec;
        findDoctor(docId, doctorName, spec);

        int totalAppointments = 0, paidAppointments = 0, unpaidAppointments = 0, seniorAppointments = 0;
        {
            std::ifstream infile("appointments.txt");
            std::string line;
            while (std::getline(infile, line)) {
                line = cleanField(line);
                if (isBlankLine(line)) continue;
                std::stringstream ss(line);
                std::string appId, patId, docIdField, slot, paid, status;
                std::getline(ss, appId, ',');
                std::getline(ss, patId, ',');
                std::getline(ss, docIdField, ',');
                std::getline(ss, slot, ',');
                std::getline(ss, paid, ',');
                std::getline(ss, status, ',');
                if (docIdField != docId) continue;
                totalAppointments++;
                if (paid == "Paid") paidAppointments++; else unpaidAppointments++;
                if (status.find("Senior Citizen Priority") != std::string::npos) seniorAppointments++;
            }
        }

        int prescriptionsWritten = 0, pendingLabTests = 0;
        {
            std::ifstream infile("prescriptions.txt");
            std::string line;
            while (std::getline(infile, line)) {
                line = cleanField(line);
                if (isBlankLine(line)) continue;
                std::stringstream ss(line);
                std::string patId, docIdField, meds, labTest, followUp;
                std::getline(ss, patId, ',');
                std::getline(ss, docIdField, ',');
                std::getline(ss, meds, ',');
                std::getline(ss, labTest, ',');
                std::getline(ss, followUp, ',');
                if (docIdField != docId) continue;
                prescriptionsWritten++;
                if (!labTest.empty() && labTest != "None" && labTest != "none" && !hasLabReport(patId)) pendingLabTests++;
            }
        }

        json res = {
            {"doctorId", docId},
            {"doctorName", doctorName},
            {"specialization", spec},
            {"totalAppointments", totalAppointments},
            {"paidAppointments", paidAppointments},
            {"unpaidAppointments", unpaidAppointments},
            {"seniorAppointments", seniorAppointments},
            {"consultationFee", CONSULTATION_FEE},
            {"totalRevenue", paidAppointments * CONSULTATION_FEE},
            {"prescriptionsWritten", prescriptionsWritten},
            {"pendingLabTests", pendingLabTests}
        };
        return crow::response(200, res.dump());
    });

    // =========================
    // Prescriptions
    // =========================
    CROW_ROUTE(app, "/api/prescriptions")
    ([](){
        std::ifstream infile("prescriptions.txt");
        json prescriptions = json::array();
        std::string line;
        while (std::getline(infile, line)) {
            line = cleanField(line);
            if (isBlankLine(line)) continue;
            std::stringstream ss(line);
            std::string patId, docId, meds, labTest, followUp;
            std::getline(ss, patId, ',');
            std::getline(ss, docId, ',');
            std::getline(ss, meds, ',');
            std::getline(ss, labTest, ',');
            std::getline(ss, followUp, ',');
            if (!patId.empty()) {
                prescriptions.push_back({
                    {"patientId", patId},
                    {"doctorId", docId},
                    {"medication", meds},
                    {"labTestOrder", labTest},
                    {"followUpDate", followUp}
                });
            }
        }
        return crow::response{prescriptions.dump()};
    });

    // POST: Add prescription
    CROW_ROUTE(app, "/api/prescriptions/add").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::ofstream file("prescriptions.txt", std::ios::app);
            if (!file.is_open()) return crow::response(500, "Unable to open prescriptions.txt");
            file << body["patientId"].get<std::string>() << ","
                 << body["doctorId"].get<std::string>() << ","
                 << sanitizeCsvField(body["medication"].get<std::string>()) << ","
                 << sanitizeCsvField(body["labTestOrder"].get<std::string>()) << ","
                 << sanitizeCsvField(body["followUpDate"].get<std::string>()) << "\n";
            return crow::response(200, "Prescription added successfully!");
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Lab Reports
    // =========================

    // Pending lab orders = prescriptions with a lab test ordered that don't
    // already have a report on file. This excludes tests already completed.
    CROW_ROUTE(app, "/api/lab/pending")
    ([](){
        std::ifstream infile("prescriptions.txt");
        json pending = json::array();
        std::string line;
        while (std::getline(infile, line)) {
            line = cleanField(line);
            if (isBlankLine(line)) continue;
            std::stringstream ss(line);
            std::string patId, docId, meds, labTest, followUp;
            std::getline(ss, patId, ',');
            std::getline(ss, docId, ',');
            std::getline(ss, meds, ',');
            std::getline(ss, labTest, ',');
            std::getline(ss, followUp, ',');
            if (patId.empty()) continue;
            if (labTest.empty() || labTest == "None" || labTest == "none") continue;
            if (hasLabReport(patId)) continue; // already reported - not pending anymore
            pending.push_back({
                {"patientId", patId},
                {"doctorId", docId},
                {"medication", meds},
                {"labTestOrder", labTest},
                {"followUpDate", followUp}
            });
        }
        return crow::response{pending.dump()};
    });

    CROW_ROUTE(app, "/api/lab/reports")
    ([](){
        std::ifstream infile("lab_reports.txt");
        json reports = json::array();
        std::string line;
        while (std::getline(infile, line)) {
            line = cleanField(line);
            if (isBlankLine(line)) continue;
            std::stringstream ss(line);
            std::string patId, findings, status;
            std::getline(ss, patId, ',');
            std::getline(ss, findings, ',');
            std::getline(ss, status, ',');
            if (!patId.empty()) {
                reports.push_back({
                    {"patientId", patId},
                    {"findings", findings},
                    {"status", status}
                });
            }
        }
        return crow::response{reports.dump()};
    });

    // POST: Add lab report
    CROW_ROUTE(app, "/api/lab/reports/add").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);
            std::ofstream file("lab_reports.txt", std::ios::app);
            if (!file.is_open()) return crow::response(500, "Unable to open lab_reports.txt");
            file << body["patientId"].get<std::string>() << ","
                 << sanitizeCsvField(body["findings"].get<std::string>()) << ",Complete\n";
            return crow::response(200, "Lab report submitted successfully!");
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Start server
    // =========================
    app.port(18080).multithreaded().run();
}