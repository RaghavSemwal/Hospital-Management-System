#include "crow.h"
#include "json.hpp"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

using json = nlohmann::json;

// Utility: Convert patients.txt into JSON
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
    // GET: List all patients
    // =========================
    CROW_ROUTE(app, "/api/patients")
    ([](){
        std::ifstream infile("patients.txt");
        if (!infile.is_open()) {
            return crow::response(500, "Database file missing or inaccessible.");
        }
        return crow::response{loadPatients().dump()};
    });

    // =========================
    // POST: Add new patient
    // =========================
    CROW_ROUTE(app, "/api/patients/add").methods("POST"_method)
    ([](const crow::request& req){
        try {
            auto body = json::parse(req.body);

            std::ofstream file("patients.txt", std::ios::app);
            if (!file.is_open()) {
                return crow::response(500, "Unable to open patients.txt");
            }

            file << body["id"].get<std::string>() << ","
                 << body["name"].get<std::string>() << ","
                 << body["age"].get<std::string>() << ","
                 << body["ailment"].get<std::string>() << ","
                 << body["phone"].get<std::string>() << "\n";
            file.close();

            return crow::response(200, "Patient added successfully!");
        } catch (...) {
            return crow::response(400, "Invalid JSON payload");
        }
    });

    // =========================
    // PUT: Update patient by ID
    // =========================
    CROW_ROUTE(app, "/api/patients/<string>").methods("PUT"_method)
    ([](const crow::request& req, std::string id){
        try {
            auto body = json::parse(req.body);

            std::ifstream infile("patients.txt");
            if (!infile.is_open()) return crow::response(500, "Unable to open patients.txt");

            std::vector<std::string> lines;
            std::string line;
            bool updated = false;

            while (std::getline(infile, line)) {
                if (line.rfind(id + ",", 0) == 0) {
                    std::string newLine = id + "," + body["name"].get<std::string>() + "," +
                                          body["age"].get<std::string>() + "," +
                                          body["ailment"].get<std::string>() + "," +
                                          body["phone"].get<std::string>();
                    lines.push_back(newLine);
                    updated = true;
                } else {
                    lines.push_back(line);
                }
            }
            infile.close();

            std::ofstream outfile("patients.txt");
            for (auto& l : lines) outfile << l << "\n";
            outfile.close();

            if (updated) return crow::response(200, "Patient updated successfully!");
            else return crow::response(404, "Patient not found");
        } catch (...) {
            return crow::response(400, "Invalid JSON payload");
        }
    });

    // =========================
    // DELETE: Remove patient by ID
    // =========================
    CROW_ROUTE(app, "/api/patients/<string>").methods("DELETE"_method)
    ([](const crow::request& req, std::string id){
        std::ifstream infile("patients.txt");
        if (!infile.is_open()) return crow::response(500, "Unable to open patients.txt");

        std::vector<std::string> lines;
        std::string line;
        bool found = false;

        while (std::getline(infile, line)) {
            if (line.rfind(id + ",", 0) == 0) {
                found = true; // skip this line
            } else {
                lines.push_back(line);
            }
        }
        infile.close();

        std::ofstream outfile("patients.txt");
        for (auto& l : lines) outfile << l << "\n";
        outfile.close();

        if (found) return crow::response(200, "Patient deleted successfully!");
        else return crow::response(404, "Patient not found");
    });

    // =========================
    // Start server
    // =========================
    app.port(18080).multithreaded().run();
}
