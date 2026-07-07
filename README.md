# 🏥 Central HMS — Hospital Management System

Created by Raghav Semwal , Abhishek Sharma and Simar Singh . 
A lightweight hospital management system with three portals — **Patient**, **Doctor**, and **Lab** — backed by a C++ REST API ([Crow](https://github.com/CrowCpp/Crow) framework) and a plain HTML/CSS/JS frontend. Data is persisted in flat `.txt` (CSV-style) files, no database required.

---

## ✨ Features

- **Patient Portal** — register, pick a specialist, check live slot availability (max 3 patients/slot), simulate payment, and get a booking receipt with a Patient ID + Appointment ID.
- **Doctor Portal** — login, view your appointments, a senior-citizen-priority-sorted active queue, your written prescriptions, revenue/performance analytics, and a full patient directory. Write new prescriptions (with optional lab test order).
- **Lab Portal** — login, view pending lab orders (prescriptions with a lab test that hasn't been reported yet), view submitted reports, submit new findings, and browse the patient directory.

---

## 🗂️ Project Structure

```
.
├── main_server.cpp        # Crow HTTP server + REST API + CSV file I/O
├── crow.h                 # Crow framework (header-only, requires crow/ subfolder + Boost.Asio)
├── json.hpp               # nlohmann/json (header-only)
├── Patient.h / Doctor.h / Appointment.h   # Simple data model classes
│
├── frontend/               # served as static files by the C++ server
│   ├── index.html          # Landing page (links to the 3 portals)
│   ├── patient.html
│   ├── doctor.html
│   ├── lab.html
│   ├── app.js               # All client-side fetch()/DOM logic
│   └── style.css            # Shared visual theme
│
└── data/ (flat files read/written by the server at runtime)
    ├── patients.txt         # id,name,age,ailment,phone
    ├── doctors.txt          # id,name,specialization,password
    ├── appointments.txt     # appointmentId,patientId,doctorId,timeSlot,feePaid,status
    ├── prescriptions.txt    # patientId,doctorId,medication,labTestOrder,followUpDate
    ├── lab_staff.txt        # labId,name,password
    └── lab_reports.txt      # patientId, findings, status
```

> **Note:** `main_server.cpp` expects the frontend files inside a `frontend/` folder next to the executable, and the six `.txt` data files in the same working directory as the executable. Adjust paths in `main_server.cpp` if your layout differs.

---

## ⚙️ Requirements

- A C++17 (or later) compiler (e.g. `g++`)
- [Boost.Asio](https://www.boost.org/doc/libs/release/libs/asio/) (Crow's networking dependency)
- The full [Crow](https://github.com/CrowCpp/Crow) header set (this repo's `crow.h` pulls in headers from a `crow/` subfolder — make sure that folder is present alongside it)

---

## 🚀 Build & Run

```bash
# from the project root
g++ -std=c++17 main_server.cpp -o hospital_server -lpthread -lboost_system

./hospital_server
```

The server starts on **http://localhost:18080**. Open that URL in a browser to reach the landing page.

> Rebuild `hospital_server` (or `hospital_server.exe` on Windows) any time `main_server.cpp` changes.

---

## 🔑 Test Logins

| Portal | ID            | Password     |
|--------|---------------|--------------|
| Doctor | 101–105       | `doc<ID>pass` (e.g. `doc101pass`) |
| Lab    | L001, L002    | `lab001pass`, `lab002pass` |

---

## 📡 API Overview

| Method | Endpoint                     | Purpose |
|--------|-------------------------------|---------|
| GET    | `/api/patients`               | List all patients |
| POST   | `/api/patients/add`            | Add a patient |
| GET    | `/api/doctors`                 | List all doctors (no passwords) |
| POST   | `/api/doctor/login`            | Doctor login |
| POST   | `/api/lab/login`               | Lab staff login |
| GET    | `/api/slots?doctorId=`         | Slot availability for a doctor |
| POST   | `/api/appointments/book`       | Book an appointment (registers patient + payment simulation) |
| GET    | `/api/appointments`            | List all appointments |
| GET    | `/api/doctor/queue?doctorId=`  | Senior-priority-sorted active queue for a doctor |
| GET    | `/api/doctor/analytics?doctorId=` | Revenue & performance stats for a doctor |
| GET    | `/api/prescriptions`           | List all prescriptions |
| POST   | `/api/prescriptions/add`       | Add a prescription |
| GET    | `/api/lab/pending`             | Prescriptions with an unreported lab test |
| GET    | `/api/lab/reports`             | List all submitted lab reports |
| POST   | `/api/lab/reports/add`         | Submit a lab report |

---

## 🧾 Data File Format Notes

- All data files are comma-separated, one record per line, **no header row**.
- Save these files with **Unix (`\n`) line endings**, not Windows (`\r\n`). The server is defensive against `\r\n` (it strips stray `\r` and skips blank lines), but keeping files Unix-formatted avoids any ambiguity if you edit them by hand.
- Avoid typing literal commas into free-text fields (medication, ailment, findings, etc.) — the server auto-replaces commas with semicolons on write to protect column alignment, but it's cleaner to avoid them at the source.

---

## 🩹 Known Fixes / Changelog

- **Login failures for doctors 101–104 and lab staff L001** — caused by trailing `\r` characters (Windows line endings) silently corrupting the last CSV column (the password) on every line except the file's last line. Fixed with a `cleanField()`/`isBlankLine()` guard applied to every file read.
- **Phantom blank records** — trailing blank lines in the `.txt` files were being parsed as real (non-empty) records. Fixed by the same guard.
- **Analytics/lab-order data drift** — free-text fields containing a comma (e.g. "Paracetamol, twice daily") would shift every subsequent CSV column. Fixed with a `sanitizeCsvField()` helper applied before writing.
- **Misaligned "Write Prescription" and Lab Login forms** — these were the only forms not wrapped in a `<fieldset>`, so the shared form styling never applied to them. Fixed by wrapping all forms consistently.
- **Patient directory unreachable from the UI** — `/api/patients` existed on the backend but no page called it. Added a "View All Patients" button to the Doctor and Lab dashboards.

---

## 📄 License

Internal/educational project 
