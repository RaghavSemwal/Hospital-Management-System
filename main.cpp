#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib> 
#include <ctime>   
#include "Patient.h"
#include "Doctor.h"
#include "Appointment.h"

// ================= SECURITY HELPERS =================

// Terminate program cleanly on invalid input
void terminateOnError(const std::string& msg) {
    std::cout << "\n=====================================\n";
    std::cout << "   ❌ SECURITY ALERT: INVALID INPUT   \n";
    std::cout << "=====================================\n";
    std::cout << msg << "\n";
    std::cout << "Program will terminate. Please restart and try again.\n";
    exit(0);
}

// Safe integer input wrapper with boundary checking
int safeInputInt(const std::string& prompt, int minVal, int maxVal) {
    int value;
    std::cout << prompt;
    if (!(std::cin >> value)) {
        terminateOnError("Non-numeric input detected!");
    }
    if (value < minVal || value > maxVal) {
        terminateOnError("Input out of allowed range!");
    }
    return value;
}

// ================= FILE SAVE FUNCTIONS =================

// Save Patient to patients.txt (Appends cleanly to file)
void savePatientToDisk(const Patient& p) {
    std::ofstream outFile("patients.txt", std::ios::app); 
    if (outFile) {
        outFile << p.id << "," << p.name << "," << p.age << "," << p.ailment << "," << p.phoneNumber << "\n";
        outFile.close();
    }
}

// Save Appointment to appointments.txt (Appends cleanly to file)
void saveAppointmentToDisk(const Appointment& a) {
    std::ofstream outFile("appointments.txt", std::ios::app);
    if (outFile) {
        outFile << a.appointmentId << "," << a.patientId << "," << a.doctorId << "," << a.timeSlot << "," << (a.feePaid ? "Paid" : "Unpaid") << "," << a.status << "\n";
        outFile.close();
    }
}

// === CAPACITY TRACKING: Counts existing bookings for a slot (Max Capacity = 3) ===
int getSlotBookingCount(int doctorId, const std::string& slot) {
    std::ifstream inFile("appointments.txt");
    if (!inFile) return 0; 

    int count = 0;
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string appIdStr, patIdStr, docIdStr, slotStr;

        std::getline(ss, appIdStr, ',');
        std::getline(ss, patIdStr, ',');
        std::getline(ss, docIdStr, ',');
        std::getline(ss, slotStr, ',');

        if (!docIdStr.empty() && std::stoi(docIdStr) == doctorId && slotStr == slot) {
            count++;
        }
    }
    inFile.close();
    return count;
}

// ================= VIEW BOOKING STATUS =================
void viewBookingStatus() {
    std::cout << "\n--- MY APPOINTMENT STATUS VIEWER ---\n";
    int inputAppId = safeInputInt("Enter your 5-digit Appointment ID: ", 10000, 99999);

    std::ifstream inFile("appointments.txt");
    if (!inFile) {
        std::cout << "❌ Database empty or no appointments recorded yet!\n";
        return;
    }

    bool recordFound = false;
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string appIdStr, patIdStr, docIdStr, slotStr, paidStr, statusStr;

        std::getline(ss, appIdStr, ',');
        std::getline(ss, patIdStr, ',');
        std::getline(ss, docIdStr, ',');
        std::getline(ss, slotStr, ',');
        std::getline(ss, paidStr, ',');
        std::getline(ss, statusStr, ',');

        if (!appIdStr.empty() && std::stoi(appIdStr) == inputAppId) {
            recordFound = true;
            std::cout << "\n==================================================\n";
            std::cout << "         OFFICIAL APPOINTMENT VERIFICATION        \n";
            std::cout << "==================================================\n";
            std::cout << " Appointment ID   : " << appIdStr << "\n";
            std::cout << " Registered Pat ID: " << patIdStr << "\n";
            std::cout << " Assigned Doc ID  : " << docIdStr << "\n";
            std::cout << " Scheduled Slot   : " << slotStr << "\n";
            std::cout << " Payment Ledger   : " << paidStr << "\n";
            std::cout << " Current Status   : " << statusStr << "\n";
            std::cout << "==================================================\n";
            break;
        }
    }
    inFile.close();

    if (!recordFound) {
        terminateOnError("No booking records match the entered Appointment ID.");
    }
}

// ================= BOOKING ENGINE =================
void bookAppointment(const std::vector<Doctor>& doctors) {
    std::string name, ailment, phone, slot;
    int age, docChoice, slotChoice;
    bool validDoctor = false;
    std::string selectedDocName = "";

    std::cout << "\n--- PATIENT INTAKE PORTAL ---\n";
    std::cin.ignore(); 
    std::cout << "Enter Patient Name: ";
    std::getline(std::cin, name);

    age = safeInputInt("Enter Age: ", 1, 120);

    std::cin.ignore();
    std::cout << "Enter Primary Ailment: ";
    std::getline(std::cin, ailment);
    std::cout << "Enter Mobile Number: ";
    std::getline(std::cin, phone);

    // Display Doctors dynamically from system vector
    std::cout << "\n==================================================\n";
    std::cout << "           AVAILABLE MEDICAL SPECIALISTS          \n";
    std::cout << "==================================================\n";
    for (const auto& doc : doctors) {
        std::cout << " ID: " << doc.id << " | " << doc.name << " (" << doc.specialization << ")\n";
    }
    std::cout << "==================================================\n";
    
    docChoice = safeInputInt("Enter Doctor ID to choose: ", 101, 105);
    for (const auto& doc : doctors) {
        if (doc.id == docChoice) {
            validDoctor = true;
            selectedDocName = doc.name;
            break;
        }
    }
    if (!validDoctor) {
        terminateOnError("Invalid Doctor ID entered!");
    }

    // Capacity-3 Tracking System 
    std::string s1 = "09:00 AM - 10:00 AM";
    std::string s2 = "11:00 AM - 12:00 PM";
    std::string s3 = "02:00 PM - 03:00 PM";

    bool slotSelected = false;
    while (!slotSelected) {
        int c1 = getSlotBookingCount(docChoice, s1);
        int c2 = getSlotBookingCount(docChoice, s2);
        int c3 = getSlotBookingCount(docChoice, s3);

        std::cout << "\nChecking live availability for " << selectedDocName << " (Max 3 per slot)...\n";
        std::cout << "1. " << s1 << " (" << c1 << "/3 Booked)" << (c1 >= 3 ? " [❌ FULL]" : " [✔️ AVAILABLE]") << "\n";
        std::cout << "2. " << s2 << " (" << c2 << "/3 Booked)" << (c2 >= 3 ? " [❌ FULL]" : " [✔️ AVAILABLE]") << "\n";
        std::cout << "3. " << s3 << " (" << c3 << "/3 Booked)" << (c3 >= 3 ? " [❌ FULL]" : " [✔️ AVAILABLE]") << "\n";

        slotChoice = safeInputInt("Selection (1-3): ", 1, 3);

        if (slotChoice == 1) slot = s1;
        else if (slotChoice == 2) slot = s2;
        else slot = s3;

        if (getSlotBookingCount(docChoice, slot) >= 3) {
            terminateOnError("That slot has hit its maximum capacity of 3 patients!");
        } else {
            slotSelected = true;
        }
    }

    // Process Booking & Gateway
    std::cout << "\n[PAYMENT GATEWAY] Processing fee of 500 INR for " << selectedDocName << "...\n";
    std::cout << "[PAYMENT GATEWAY] Transaction Authorized Successfully!\n";
    bool feePaid = true;

    // Age Group Flagging
    std::string status = "Confirmed";
    if (age > 60) {
        status = "Confirmed (Senior Citizen Priority)";
        std::cout << "💡 Senior Citizen status detected. Priority status tag embedded in booking record.\n";
    }

    int patientId = rand() % 9000 + 1000;      
    int appointmentId = rand() % 90000 + 10000; 

    Patient newPatient(patientId, name, age, ailment, phone);
    Appointment newAppointment(appointmentId, patientId, docChoice, slot, feePaid, status);

    savePatientToDisk(newPatient);
    saveAppointmentToDisk(newAppointment);

    std::cout << "\n✔️ Appointment Successfully Scheduled!\n";
    std::cout << "Your Patient ID is: " << patientId << "\n";
    std::cout << "Your Appointment ID is: " << appointmentId << "\n";
}

// ================= CENTRAL CONTROLLER =================
int main() {
    srand(time(0)); 
    
    std::vector<Doctor> hospitalDoctors;
    hospitalDoctors.push_back(Doctor(101, "Dr. Abhishek", "Cardiologist"));
    hospitalDoctors.push_back(Doctor(102, "Dr. Sharma", "General Physician"));
    hospitalDoctors.push_back(Doctor(103, "Dr. Simar", "Radiologist"));
    hospitalDoctors.push_back(Doctor(104, "Dr. Priya Kaur", "Pediatrician"));
    hospitalDoctors.push_back(Doctor(105, "Dr. Malhotra", "Neurologist"));

    while (true) {
        std::cout << "\n=====================================\n";
        std::cout << "     HMS SECURE PATIENT MODULE       \n";
        std::cout << "=====================================\n";
        std::cout << "1. Book Appointment / Register Profile\n";
        std::cout << "2. Track My Booking Receipt Status\n"; 
        std::cout << "3. Exit Subsystem Application\n";
        
        int menuChoice = safeInputInt("Selection (1-3): ", 1, 3);

        if (menuChoice == 3) {
            std::cout << "\nExiting Subsystem cleanly. All disk data sequences sustained.\n";
            break;
        }
        else if (menuChoice == 1) {
            bookAppointment(hospitalDoctors); 
        } 
        else if (menuChoice == 2) {
            viewBookingStatus(); 
        }
    }
    return 0;
}