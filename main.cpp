#include <iostream>
#include <fstream>
#include <vector>
#include "Patient.h"
#include "Doctor.h"
#include "Appointment.h"

// Function to save patients to patients.txt
void savePatientsToFile(const std::vector<Patient>& patients) {
    std::ofstream outFile("patients.txt"); 
    if (!outFile) {
        std::cerr << "Error opening file!" << std::endl;
        return;
    }
    for (const auto& p : patients) {
        outFile << p.id << "," << p.name << "," << p.age << "," << p.ailment << "\n";
    }
    outFile.close();
    std::cout << "Successfully saved records to patients.txt" << std::endl;
}

int main() {
    std::vector<Patient> hospitalPatients;

    std::cout << "--- HMS Core Engine Active ---" << std::endl;

    // Adding sample data
    hospitalPatients.push_back(Patient(1, "John Doe", 29, "Fever"));
    hospitalPatients.push_back(Patient(2, "Jane Smith", 42, "Migraine"));

    // Save to disk
    savePatientsToFile(hospitalPatients);

    return 0;
}