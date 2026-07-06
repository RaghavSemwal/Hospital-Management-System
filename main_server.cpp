#include "crow.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using json = nlohmann::json;

// =========================
// Utility: Load Patients
// =========================
json loadPatients() {
    std::ifstream infile("patients.txt");
    json patients = json::array();
    std::string line;

    while (std::getline(infile, line)) {
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

int main() {
    crow::SimpleApp app;

    // =========================
    // Serve frontend files
    // =========================
    CROW_ROUTE(app, "/")([](){
        std::ifstream file("frontend/index.html");
        if (!file.is_open()) return crow::response(404, "index.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        return crow::response(buffer.str());
    });

    CROW_ROUTE(app, "/style.css")([](){
        std::ifstream file("frontend/style.css");
        if (!file.is_open()) return crow::response(404, "style.css not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "text/css");
        return res;
    });

    CROW_ROUTE(app, "/app.js")([](){
        std::ifstream file("frontend/app.js");
        if (!file.is_open()) return crow::response(404, "app.js not found");
        std::stringstream buffer; buffer << file.rdbuf();
        crow::response res(buffer.str());
        res.add_header("Content-Type", "application/javascript");
        return res;
    });

    CROW_ROUTE(app, "/patient.html")([](){
        std::ifstream file("frontend/patient.html");
        if (!file.is_open()) return crow::response(404, "patient.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        return crow::response(buffer.str());
    });

    CROW_ROUTE(app, "/doctor.html")([](){
        std::ifstream file("frontend/doctor.html");
        if (!file.is_open()) return crow::response(404, "doctor.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        return crow::response(buffer.str());
    });

    CROW_ROUTE(app, "/lab.html")([](){
        std::ifstream file("frontend/lab.html");
        if (!file.is_open()) return crow::response(404, "lab.html not found");
        std::stringstream buffer; buffer << file.rdbuf();
        return crow::response(buffer.str());
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
            file << body["id"].get<std::string>() << ","
                 << body["name"].get<std::string>() << ","
                 << body["age"].get<std::string>() << ","
                 << body["ailment"].get<std::string>() << ","
                 << body["phone"].get<std::string>() << "\n";
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
    // Prescriptions
    // =========================
    CROW_ROUTE(app, "/api/prescriptions")
    ([](){
        std::ifstream infile("prescriptions.txt");
        json prescriptions = json::array();
        std::string line;
        while (std::getline(infile, line)) {
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
                 << body["medication"].get<std::string>() << ","
                 << body["labTestOrder"].get<std::string>() << ","
                 << body["followUpDate"].get<std::string>() << "\n";
            return crow::response(200, "Prescription added successfully!");
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Lab Reports
    // =========================
    CROW_ROUTE(app, "/api/lab/reports")
    ([](){
        std::ifstream infile("lab_reports.txt");
        json reports = json::array();
        std::string line;
        while (std::getline(infile, line)) {
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
                 << body["findings"].get<std::string>() << ",Complete\n";
            return crow::response(200, "Lab report submitted successfully!");
        } catch (...) { return crow::response(400, "Invalid JSON payload"); }
    });

    // =========================
    // Start server
    // =========================
    app.port(18080).multithreaded().run();
}