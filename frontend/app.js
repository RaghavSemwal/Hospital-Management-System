// Utility: render JSON data into a table
function renderTable(data, headers, targetId) {
  let html = "<table><thead><tr>";
  headers.forEach(h => html += `<th>${h}</th>`);
  html += "</tr></thead><tbody>";
  data.forEach(row => {
    html += "<tr>";
    headers.forEach(h => html += `<td>${row[h] !== undefined ? row[h] : ""}</td>`);
    html += "</tr>";
  });
  html += "</tbody></table>";
  const contentDiv = document.getElementById(targetId) ||
                     document.getElementById("content") ||
                     document.getElementById("doctorContent") ||
                     document.getElementById("labContent");
  if (contentDiv) contentDiv.innerHTML = html;
}

// ==================== PATIENT PORTAL: BOOKING FLOW ====================
let selectedSlot = null;

document.addEventListener("DOMContentLoaded", () => {
  const doctorSelect = document.getElementById("doctorSelect");
  const bookingForm = document.getElementById("bookingForm");
  if (!doctorSelect || !bookingForm) return; // not on patient.html

  // Load specialists list
  fetch("/api/doctors")
    .then(res => res.json())
    .then(doctors => {
      doctors.forEach(doc => {
        const opt = document.createElement("option");
        opt.value = doc.id;
        opt.textContent = `${doc.name} (${doc.specialization}) - ID ${doc.id}`;
        doctorSelect.appendChild(opt);
      });
    })
    .catch(err => console.error(err));

  const checkSlotsBtn = document.getElementById("checkSlotsBtn");
  checkSlotsBtn.addEventListener("click", () => {
    const doctorId = doctorSelect.value;
    if (!doctorId) { alert("Please select a doctor first."); return; }

    fetch(`/api/slots?doctorId=${encodeURIComponent(doctorId)}`)
      .then(res => res.json())
      .then(data => {
        selectedSlot = null;
        document.getElementById("confirmBookingBtn").disabled = true;
        const slotFieldset = document.getElementById("slotFieldset");
        const slotOptions = document.getElementById("slotOptions");
        slotOptions.innerHTML = "";
        data.slots.forEach(s => {
          const label = document.createElement("label");
          label.style.display = "block";
          const radio = document.createElement("input");
          radio.type = "radio";
          radio.name = "timeSlot";
          radio.value = s.slot;
          radio.disabled = !s.available;
          radio.addEventListener("change", () => {
            selectedSlot = s.slot;
            document.getElementById("confirmBookingBtn").disabled = false;
          });
          label.appendChild(radio);
          label.append(` ${s.slot} — ${s.booked}/${s.capacity} booked ${s.available ? "✔ Available" : "✘ Full"}`);
          slotOptions.appendChild(label);
        });
        slotFieldset.style.display = "block";
      })
      .catch(err => console.error(err));
  });

  bookingForm.addEventListener("submit", e => {
    e.preventDefault();
    if (!selectedSlot) { alert("Please check availability and choose a time slot first."); return; }

    const formData = Object.fromEntries(new FormData(bookingForm).entries());
    const payload = {
      name: formData.name,
      age: formData.age,
      ailment: formData.ailment,
      phone: formData.phone,
      doctorId: formData.doctorId,
      timeSlot: selectedSlot,
      payNow: formData.payNow === "yes"
    };

    fetch("/api/appointments/book", {
      method: "POST",
      headers: {"Content-Type": "application/json"},
      body: JSON.stringify(payload)
    })
      .then(res => res.json().then(body => ({status: res.status, body})))
      .then(({status, body}) => {
        if (status === 200 && body.success) {
          bookingForm.style.display = "none";
          const receipt = document.getElementById("receipt");
          receipt.style.display = "block";
          receipt.innerHTML = `
            <h3>✔ Booking Confirmed — Receipt</h3>
            <table>
              <tr><th>Appointment ID</th><td>${body.appointmentId}</td></tr>
              <tr><th>Patient ID</th><td>${body.patientId}</td></tr>
              <tr><th>Patient Name</th><td>${body.patientName}</td></tr>
              <tr><th>Doctor</th><td>${body.doctorName} (${body.specialization})</td></tr>
              <tr><th>Time Slot</th><td>${body.timeSlot}</td></tr>
              <tr><th>Consultation Fee</th><td>₹${body.fee}</td></tr>
              <tr><th>Payment Status</th><td>${body.feePaid}</td></tr>
              <tr><th>Appointment Status</th><td>${body.status}</td></tr>
            </table>
            <p>Please save your Appointment ID and Patient ID for future reference.</p>
          `;
        } else {
          alert(body.message || "Booking failed");
        }
      })
      .catch(err => console.error(err));
  });
});

// ==================== DOCTOR PORTAL ====================
let currentDoctorId = null;

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
          currentDoctorId = data.doctorId;
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
  if (!currentDoctorId) { alert("Please log in first."); return; }
  fetch('/api/appointments')
    .then(res => res.json())
    .then(data => {
      const mine = data.filter(a => a.doctorId === currentDoctorId);
      renderTable(mine, ["appointmentId","patientId","doctorId","timeSlot","feePaid","status"], "doctorContent");
    })
    .catch(err => console.error(err));
}

function viewDoctorPrescriptions() {
  if (!currentDoctorId) { alert("Please log in first."); return; }
  fetch('/api/prescriptions')
    .then(res => res.json())
    .then(data => {
      const mine = data.filter(p => p.doctorId === currentDoctorId);
      renderTable(mine, ["patientId","doctorId","medication","labTestOrder","followUpDate"], "doctorContent");
    })
    .catch(err => console.error(err));
}

// Senior-citizen-priority sorted appointment queue for the logged-in doctor.
// Patients already prescribed/treated by this doctor are excluded from the active queue.
function viewDoctorQueue() {
  if (!currentDoctorId) { alert("Please log in first."); return; }
  fetch(`/api/doctor/queue?doctorId=${encodeURIComponent(currentDoctorId)}`)
    .then(res => res.json())
    .then(data => {
      const el = document.getElementById("doctorContent");
      if (data.queue.length === 0) {
        el.innerHTML = `<p>No active patients waiting. ${data.treated.length ? `(${data.treated.length} already treated)` : ""}</p>`;
        return;
      }
      const rows = data.queue.map(q => ({
        ...q,
        priority: q.priority ? "⭐ Senior Priority" : "Standard"
      }));
      renderTable(rows, ["appointmentId","patientId","patientName","age","timeSlot","feePaid","status","priority"], "doctorContent");
      if (data.treated.length) {
        el.insertAdjacentHTML("beforeend", `<p class="muted-note">${data.treated.length} patient(s) already treated and removed from the active queue.</p>`);
      }
    })
    .catch(err => console.error(err));
}

// Revenue & performance analytics dashboard for the logged-in doctor
function viewDoctorAnalytics() {
  if (!currentDoctorId) { alert("Please log in first."); return; }
  fetch(`/api/doctor/analytics?doctorId=${encodeURIComponent(currentDoctorId)}`)
    .then(res => res.json())
    .then(d => {
      const html = `
        <h4>📊 Performance & Revenue — ${d.doctorName} (${d.specialization})</h4>
        <div class="stats-grid">
          <div class="stat-card"><div class="stat-value">${d.totalAppointments}</div><div class="stat-label">Total Appointments</div></div>
          <div class="stat-card"><div class="stat-value">${d.paidAppointments}</div><div class="stat-label">Paid</div></div>
          <div class="stat-card"><div class="stat-value">${d.unpaidAppointments}</div><div class="stat-label">Unpaid</div></div>
          <div class="stat-card"><div class="stat-value">${d.seniorAppointments}</div><div class="stat-label">Senior Citizen Priority</div></div>
          <div class="stat-card"><div class="stat-value">₹${d.totalRevenue}</div><div class="stat-label">Total Revenue</div></div>
          <div class="stat-card"><div class="stat-value">${d.prescriptionsWritten}</div><div class="stat-label">Prescriptions Written</div></div>
          <div class="stat-card"><div class="stat-value">${d.pendingLabTests}</div><div class="stat-label">Lab Tests Ordered</div></div>
        </div>
      `;
      const el = document.getElementById("doctorContent");
      if (el) el.innerHTML = html;
    })
    .catch(err => console.error(err));
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
  fetch('/api/lab/pending')
    .then(res => res.json())
    .then(data => renderTable(data, ["patientId","doctorId","medication","labTestOrder","followUpDate"], "labContent"))
    .catch(err => console.error(err));
}

function viewLabReports() {
  fetch('/api/lab/reports')
    .then(res => res.json())
    .then(data => renderTable(data, ["patientId","findings","status"], "labContent"))
    .catch(err => console.error(err));
}

// ==================== SHARED: PATIENT DIRECTORY ====================
// Used by both the doctor and lab dashboards to look up patient details
// (id, name, age, ailment, phone) before writing a prescription or report.
function viewAllPatients(targetId) {
  fetch('/api/patients')
    .then(res => res.json())
    .then(data => renderTable(data, ["id","name","age","ailment","phone"], targetId))
    .catch(err => console.error(err));
}