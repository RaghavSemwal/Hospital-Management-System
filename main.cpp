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
void terminateOnError(const std::string& msg) {
    std::cout << "\n=====================================\n";
    std::cout << "   ❌ SECURITY ALERT: INVALID INPUT   \n";
    std::cout << "=====================================\n";
    std::cout << msg << "\n";
    std::cout << "Program will terminate. Please restart and try again.\n";
    exit(0);
}

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

// ================= RAGHAV'S SAVING MODULES =================
void savePatientToDisk(const Patient& p) {
    std::ofstream outFile("output/patients.txt", std::ios::app); 
    if (outFile) {
        outFile << p.id << "," << p.name << "," << p.age << "," << p.ailment << "," << p.phoneNumber << "\n";
        outFile.close();
    }
}

void saveAppointmentToDisk(const Appointment& a) {
    std::ofstream outFile("output/appointments.txt", std::ios::app);
    if (outFile) {
        outFile << a.appointmentId << "," << a.patientId << "," << a.doctorId << "," << a.timeSlot << "," << (a.feePaid ? "Paid" : "Unpaid") << "," << a.status << "\n";
        outFile.close();
    }
}

int getSlotBookingCount(int doctorId, const std::string& slot) {
    std::ifstream inFile("output/appointments.txt");
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

    std::ifstream inFile("output/appointments.txt");
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

// ================= RAGHAV'S PATIENT INTAKE =================
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

    std::cout << "\n[PAYMENT GATEWAY] Processing fee of 500 INR for " << selectedDocName << "...\n";
    std::cout << "[PAYMENT GATEWAY] Transaction Authorized Successfully!\n";
    bool feePaid = true;

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

// ================= ABHISHEK'S DOCTOR MODULE FEATURES =================

// 1. View Prioritized Patient Queue (With Senior Sorting)
void viewDoctorSchedule(int loggedInDocId) {
    std::ifstream inFile("output/appointments.txt");
    if (!inFile) {
        std::cout << "\n⚠️ No central appointments database detected or it is currently empty.\n";
        return;
    }

    std::string line;
    std::vector<std::string> priorityQueue;
    std::vector<std::string> regularQueue;

    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string appIdStr, patIdStr, docIdStr, slotStr, paidStr, statusStr;

        std::getline(ss, appIdStr, ',');
        std::getline(ss, patIdStr, ',');
        std::getline(ss, docIdStr, ',');
        std::getline(ss, slotStr, ',');
        std::getline(ss, paidStr, ',');
        std::getline(ss, statusStr, ',');

        if (!docIdStr.empty() && std::stoi(docIdStr) == loggedInDocId) {
            std::string patientCard = "App ID: " + appIdStr + " | Pat ID: " + patIdStr + " | Timing: " + slotStr;

            if (statusStr.find("Senior Citizen Priority") != std::string::npos) {
                priorityQueue.push_back(patientCard + " [Age Flag: HIGH PRIORITY]");
            } else {
                regularQueue.push_back(patientCard);
            }
        }
    }
    inFile.close();

    std::cout << "\n==================================================\n";
    std::cout << "         LIVE CLINICAL DAILY APPOINTMENT QUEUE      \n";
    std::cout << "==================================================\n";

    if (priorityQueue.empty() && regularQueue.empty()) {
        std::cout << " 💤 You have no scheduled patients booked for today.\n";
        std::cout << "==================================================\n";
        return;
    }

    for (const auto& patient : priorityQueue) {
        std::cout << " ⭐ [PRIORITY QUEUE] " << patient << "\n";
    }
    for (const auto& patient : regularQueue) {
        std::cout << " 👥 [STANDARD QUEUE] " << patient << "\n";
    }
    std::cout << "==================================================\n";
}

// Helper function to search and display a patient's profile card
bool displayPatientProfile(int patId) {
    std::ifstream inFile("output/patients.txt");
    if (!inFile) return false;

    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string idStr, name, ageStr, ailment, phone;

        std::getline(ss, idStr, ',');
        std::getline(ss, name, ',');
        std::getline(ss, ageStr, ',');
        std::getline(ss, ailment, ',');
        std::getline(ss, phone, ',');

        if (!idStr.empty() && std::stoi(idStr) == patId) {
            std::cout << "\n------------------------------------------------\n";
            std::cout << "📋 CLINICAL EHR PROFILE RETRIEVED:\n";
            std::cout << "Patient Name   : " << name << "\n";
            std::cout << "Patient Age    : " << ageStr << " Years Old\n";
            std::cout << "Initial Ailment: " << ailment << "\n";
            std::cout << "Contact Number : " << phone << "\n";
            std::cout << "------------------------------------------------\n";
            inFile.close();
            return true;
        }
    }
    inFile.close();
    return false;
}

// 2. Treat Patient (Pull Patient Profile + Write Prescription)
void prescribePatient(int loggedInDocId) {
    int targetPatId;
    std::string medication, labTestOrder, followUpDate;

    std::cout << "\n--- CLINICAL DIAGNOSTIC RADAR ---\n";
    std::cout << "Enter the Patient ID you want to treat: ";
    std::cin >> targetPatId;
    std::cin.ignore(); 

    // Search and display their details from Raghav's file first
    if (!displayPatientProfile(targetPatId)) {
        std::cout << "⚠️ Warning: Patient ID not found in intake index. Proceeding with raw setup.\n";
    }

    std::cout << "Enter Prescription / Medication Details: ";
    std::getline(std::cin, medication);
    
    std::cout << "Order Lab Diagnostic Test (or type 'None'): ";
    std::getline(std::cin, labTestOrder);
    
    std::cout << "Enter Next Follow-up Date (DD-MM-YYYY): ";
    std::getline(std::cin, followUpDate);

    std::ofstream outFile("output/prescriptions.txt", std::ios::app);
    if (outFile) {
        outFile << targetPatId << "," << loggedInDocId << "," << medication << "," << labTestOrder << "," << followUpDate << "\n";
        outFile.close();
        
        std::cout << "\n✔️ Clinical Summary Successfully Saved to Permanent Record!\n";
        if (labTestOrder != "None" && labTestOrder != "none") {
            std::cout << "📢 Alert dispatched to Lab Core Module for test pipeline: [" << labTestOrder << "]\n";
        }
    } else {
        std::cout << "❌ Critical Error: Unable to access prescriptions disk sector.\n";
    }
}

// 3. View My Past Clinical Prescription Logs
void viewPastPrescriptions(int loggedInDocId) {
    std::ifstream inFile("output/prescriptions.txt");
    std::cout << "\n==================================================\n";
    std::cout << "         HISTORICAL CLINICAL PRESCRIPTION LOGS     \n";
    std::cout << "==================================================\n";

    if (!inFile) {
        std::cout << "📝 No prescriptions have been issued by this clinic yet.\n";
        std::cout << "==================================================\n";
        return;
    }

    std::string line;
    bool logsFound = false;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::string patId, docIdStr, meds, labTest, followUp;

        std::getline(ss, patId, ',');
        std::getline(ss, docIdStr, ',');
        std::getline(ss, meds, ',');
        std::getline(ss, labTest, ',');
        std::getline(ss, followUp, ',');

        if (!docIdStr.empty() && std::stoi(docIdStr) == loggedInDocId) {
            logsFound = true;
            std::cout << "🩺 Patient ID: " << patId 
                      << " | Meds: " << meds 
                      << " | Lab Test: " << labTest 
                      << " | Follow-up: " << followUp << "\n";
        }
    }
    inFile.close();

    if (!logsFound) {
        std::cout << "You haven't written any prescriptions yet.\n";
    }
    std::cout << "==================================================\n";
}

// 4. View My Performance Analytics & Revenue Dashboard
void viewDoctorAnalytics(int loggedInDocId) {
    std::ifstream inFile("output/appointments.txt");
    int count = 0;

    if (inFile) {
        std::string line;
        while (std::getline(inFile, line)) {
            std::stringstream ss(line);
            std::string appId, patId, docIdStr;
            std::getline(ss, appId, ',');
            std::getline(ss, patId, ',');
            std::getline(ss, docIdStr, ',');

            if (!docIdStr.empty() && std::stoi(docIdStr) == loggedInDocId) {
                count++;
            }
        }
        inFile.close();
    }

    int totalRevenue = count * 500;

    std::cout << "\n==================================================\n";
    std::cout << "     📊 CLINICAL PERFORMANCE & REVENUE DASHBOARD   \n";
    std::cout << "==================================================\n";
    std::cout << " Staff Doctor ID   : " << loggedInDocId << "\n";
    std::cout << " Total Consultations: " << count << " Patients Booked\n";
    std::cout << " Base Consultation Fee: 500 INR\n";
    std::cout << " Total Gross Revenue: " << totalRevenue << " INR\n";
    std::cout << "==================================================\n";
}

// ================= CENTRAL INTEGRATED SYSTEM CONTROLLER =================
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
        std::cout << "    CENTRAL HMS CORE INTEGRATED ENGINE   \n";
        std::cout << "=====================================\n";
        std::cout << "1. Open Patient Intake Portal Subsystem\n";
        std::cout << "2. Open Doctor Clinical Portal Subsystem\n"; 
        std::cout << "3. Track Patient Booking Receipt\n";
        std::cout << "4. Exit Core Application Systems\n";
        
        int mainChoice = safeInputInt("Selection (1-4): ", 1, 4);

        if (mainChoice == 4) {
            std::cout << "\nShutting down central engine subsystems safely.\n";
            break;
        }
        else if (mainChoice == 1) {
            bookAppointment(hospitalDoctors); 
        } 
        else if (mainChoice == 3) {
            viewBookingStatus();
        }
        else if (mainChoice == 2) {
            // DOCTOR AUTH INTERFACE
            int docLoginId = safeInputInt("\nEnter Doctor Staff ID (101-105): ", 101, 105);

            bool insideDocPortal = true;
            while (insideDocPortal) {
                std::cout << "\n🏥 DOCTOR DASHBOARD | Staff ID: [" << docLoginId << "]\n";
                std::cout << "1. View My Prioritized Patient Queue (With Senior Sorting)\n";
                std::cout << "2. Treat Patient (Pull Patient Profile + Write Prescription)\n";
                std::cout << "3. View My Past Clinical Prescription Logs\n";
                std::cout << "4. View My Performance Analytics & Revenue Dashboard\n";
                std::cout << "5. Log Out of Dash\n";
                
                int docMenuChoice = safeInputInt("Selection (1-5): ", 1, 5);

                if (docMenuChoice == 1) {
                    viewDoctorSchedule(docLoginId);
                } else if (docMenuChoice == 2) {
                    prescribePatient(docLoginId);
                } else if (docMenuChoice == 3) {
                    viewPastPrescriptions(docLoginId);
                } else if (docMenuChoice == 4) {
                    viewDoctorAnalytics(docLoginId);
                } else {
                    insideDocPortal = false;
                }
            }
        }
    }
    return 0;
}