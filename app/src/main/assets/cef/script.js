/* ================= GLOBAL ================= */
let selectedItem = null;

/* ================= ALERT ================= */
function createAlert(title, message, callback) {
    document.getElementById('alert-title').innerText = title;
    document.getElementById('alert-message').innerText = message;
    const overlay = document.getElementById('alert-overlay');
    overlay.classList.remove('hidden');

    document.getElementById('okay-button').onclick = () => {
        hideAlert();
        callback(true);
    };
    document.getElementById('cancel-button').onclick = () => {
        hideAlert();
        callback(false);
    };
}

function hideAlert() {
    document.getElementById('alert-overlay').classList.add('hidden');
}

function showAlert(eventData) {
    const eventDataJson = JSON.parse(eventData);
    createAlert(eventDataJson[1], eventDataJson[2], (response) => {
        Cef.sendEvent("alert_response", JSON.stringify([parseInt(eventDataJson[0]), response]));
    });
}

/* ================= TOAST NOTIFICATION ================= */
function createToast(type, title, message, duration = 5000) {
    const container = document.getElementById("toast-container");
    const toast = document.createElement("div");
    toast.className = `toast ${type}`;
    let icon = type === "success" ? "✔" : type === "error" ? "✖" : type === "warning" ? "⚠" : "ⓘ";

    toast.innerHTML = `
        <div class="toast-icon">${icon}</div>
        <div class="toast-content">
            <div class="toast-title">${title}</div>
            <div class="toast-message">${message}</div>
        </div>
        <div class="toast-progress"></div>
    `;
    container.appendChild(toast);
    setTimeout(() => {
        toast.classList.add("hide");
        setTimeout(() => toast.remove(), 400);
    }, duration);
}

function showNotification(eventData) {
    const data = JSON.parse(eventData);
    createToast(data[0], data[1] || "THÔNG BÁO", data[2] || "", parseInt(data[3] || 5000));
}
/* ================= EVENT REGISTRATION ================= *
Cef.registerEventCallback("alert_show", "showAlert");
Cef.registerEventCallback("notification_show", "showNotification");
