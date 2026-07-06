// Utility: render JSON data into a table
function renderTable(data, headers) {
  let html = "<table><thead><tr>";
  headers.forEach(h => html += `<th>${h}</th>`);
  html += "</tr></thead><tbody>";
  data.forEach(row => {
    html += "<tr>";
    headers.forEach(h => html += `<td>${row[h] || ""}</td>`);
    html += "</tr>";
  });
  html += "</tbody></table>";
  const contentDiv = document.getElementById("content") || 
                     document.getElementById("doctorContent") || 
                     document.getElementById("labContent");
  if (contentDiv) contentDiv.innerHTML = html;
}

// ==================== PATIENT PORTAL ====================
document.addEventListener("DOMContentLoaded", () => {
  const patientForm = document.getElementById("patientForm");
  if (patientForm) {
    patientForm.addEventListener("submit", e => {
      e.preventDefault();
      const data = Object.fromEntries(new FormData(patientForm).entries());
      fetch("/api/patients/add", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify(data)
      })
      .then(res => res.text())
      .then(msg => alert(msg))
      .catch(err => console.error(err));
    });
  }
});

// ==================== DOCTOR PORTAL ====================
document.addEventListener("DOMContentLoaded", () => {
  const doctorLoginForm = document.getElementById("doctorLoginForm");
  if (doctorLoginForm) {
    doctorLoginForm.addEventListener("submit", e => {
      e.preventDefault();
      const data = Object.fromEntries(new FormData(doctorLoginForm).entries());
      fetch("/api/doctor/login", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify(data)
      })
      .then(res => res.json().then(body => ({status: res.status, body})))
      .then(({status, body}) => {
        if (status === 200 && body.success) {
          doctorLoginForm.style.display = "none";
          document.getElementById("doctorDashboard").style.display = "block";
        } else {
          alert(body.message || "Login failed");
        }
      })
      .catch(err => console.error(err));
    });
  }

  const prescriptionForm = document.getElementById("prescriptionForm");
  if (prescriptionForm) {
    prescriptionForm.addEventListener("submit", e => {
      e.preventDefault();
      const data = Object.fromEntries(new FormData(prescriptionForm).entries());
      fetch("/api/prescriptions/add", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify(data)
      })
      .then(res => res.text())
      .then(msg => alert(msg))
      .catch(err => console.error(err));
    });
  }
});

function viewDoctorAppointments() {
  fetch('/api/appointments')
    .then(res => res.json())
    .then(data => renderTable(data, ["appointmentId","patientId","doctorId","timeSlot","feePaid","status"]))
    .catch(err => console.error(err));
}

function viewDoctorPrescriptions() {
  fetch('/api/prescriptions')
    .then(res => res.json())
    .then(data => renderTable(data, ["patientId","doctorId","medication","labTestOrder","followUpDate"]))
    .catch(err => console.error(err));
}

function viewDoctorAnalytics() {
  // Placeholder: connect to backend analytics later
  document.getElementById("doctorContent").innerHTML = "<p>Analytics dashboard coming soon...</p>";
}

// ==================== LAB PORTAL ====================
document.addEventListener("DOMContentLoaded", () => {
  const labLoginForm = document.getElementById("labLoginForm");
  if (labLoginForm) {
    labLoginForm.addEventListener("submit", e => {
      e.preventDefault();
      const data = Object.fromEntries(new FormData(labLoginForm).entries());
      fetch("/api/lab/login", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify(data)
      })
      .then(res => res.json().then(body => ({status: res.status, body})))
      .then(({status, body}) => {
        if (status === 200 && body.success) {
          labLoginForm.style.display = "none";
          document.getElementById("labDashboard").style.display = "block";
        } else {
          alert(body.message || "Login failed");
        }
      })
      .catch(err => console.error(err));
    });
  }

  const labReportForm = document.getElementById("labReportForm");
  if (labReportForm) {
    labReportForm.addEventListener("submit", e => {
      e.preventDefault();
      const data = Object.fromEntries(new FormData(labReportForm).entries());
      fetch("/api/lab/reports/add", {
        method: "POST",
        headers: {"Content-Type":"application/json"},
        body: JSON.stringify(data)
      })
      .then(res => res.text())
      .then(msg => alert(msg))
      .catch(err => console.error(err));
    });
  }
});

function viewPendingLabOrders() {
  fetch('/api/prescriptions')
    .then(res => res.json())
    .then(data => {
      const pending = data.filter(p => p.labTestOrder && p.labTestOrder.toLowerCase() !== "none");
      renderTable(pending, ["patientId","doctorId","medication","labTestOrder","followUpDate"]);
    })
    .catch(err => console.error(err));
}

function viewLabReports() {
  fetch('/api/lab/reports')
    .then(res => res.json())
    .then(data => renderTable(data, ["patientId","findings","status"]))
    .catch(err => console.error(err));
}