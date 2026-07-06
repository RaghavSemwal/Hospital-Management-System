#include "crow.h"

#include "json.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using json = nlohmann::json;

int main() {
    crow::SimpleApp app; // Initialize Crow server core

    // Root diagnostic route
    CROW_ROUTE(app, "/")([](){
        return "Central HMS Server Engine is Online!";
    });

    // ============================
    // /api/patients GET Endpoint
    // ============================
    CROW_ROUTE(app, "/api/patients")([](){
        std::ifstream file("patients.txt");   // <-- make sure this file exists
        json patientArray = json::array();

        if (!file.is_open()) {
            return crow::response(500, "Database file missing or inaccessible.");
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty()) continue;

            std::stringstream ss(line);
            std::string id, name, age, ailment, phone;

            std::getline(ss, id, ',');
            std::getline(ss, name, ',');
            std::getline(ss, age, ',');
            std::getline(ss, ailment, ',');
            std::getline(ss, phone, ',');

            json p;
            p["id"] = id;
            p["name"] = name;
            p["age"] = age;
            p["ailment"] = ailment;
            p["phone"] = phone;

            patientArray.push_back(p);
        }
        file.close();

        crow::response res;
        res.set_header("Content-Type", "application/json");
        res.body = patientArray.dump(4); // pretty-print JSON
        return res;
    });

    // Start server
    std::cout << "Starting HMS Central Server on port 18080..." << std::endl;
    app.port(18080).multithreaded().run();
}
