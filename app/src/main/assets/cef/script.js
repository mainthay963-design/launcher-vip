// ================= ALERT =================

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
    const overlay = document.getElementById('alert-overlay');
    overlay.classList.add('hidden');
}

function showAlert(eventData) {
    const eventDataJson = JSON.parse(eventData);

    const alertId = parseInt(eventDataJson[0]);
    const alertTitle = eventDataJson[1];
    const alertMessage = eventDataJson[2];

    createAlert(alertTitle, alertMessage, (response) => {
        let outgoingEventData = new Array();

        outgoingEventData.push(alertId);
        outgoingEventData.push(response);

        console.log(`Response status: ${response}`);

        Cef.sendEvent(
            "alert_response",
            JSON.stringify(outgoingEventData)
        );
    });
}

/* ================= TOAST NOTIFICATION ================= */

function createToast(type, title, message) {
    const container = document.getElementById("toast-container");

    // ĐÃ SỬA: Dọn sạch hộp chứa để xóa bỏ hoàn toàn tin nhắn cũ trước khi hiện tin mới
    container.innerHTML = ""; 

    const toast = document.createElement("div");

    toast.classList.add("toast");
    toast.classList.add(type);

    toast.innerHTML = `
        <div class="toast-title">${title}</div>
        <div class="toast-message">${message}</div>
    `;

    container.appendChild(toast);

    // Auto remove
    setTimeout(() => {
        toast.classList.add("hide");

        setTimeout(() => {
            toast.remove();
        }, 400);

    }, 5000);
}

function showNotification(eventData) {
    const data = JSON.parse(eventData);

    const type = data[0];     // Lấy loại thông báo: success, error, warning, info
    const message = data[2];  // Lấy nội dung thông báo từ chuỗi Pawn gửi qua

    // Tự động đặt lại Tiêu đề viết hoa dựa theo từng Loại (Type)
    let title = "THÔNG BÁO";
    if (type === "error") {
        title = "ERROR";
    } else if (type === "warning") {
        title = "WARNING";
    } else if (type === "info") {
        title = "INFO";
    } else if (type === "success") {
        title = "SUCCESS";
    }
    createToast(type, title, message);
}
/* ================= INVENTORY ================= */

document.getElementById('close-inv-btn').onclick = () => {
    document.getElementById('inventory-container').classList.add('hidden');
    // Gửi tín hiệu về cho Pawn biết người chơi đã đóng túi đồ (để nhả chuột/freeze)
    Cef.sendEvent("inventory_action", JSON.stringify(["close", 0]));
};

function renderInventory(eventData) {
    const data = JSON.parse(eventData);
    const action = data[0]; // "show" hoặc "hide"
    
    const container = document.getElementById('inventory-container');

    if (action === "hide") {
        container.classList.add('hidden');
        return;
    }

    // Mở túi đồ
    container.classList.remove('hidden');
    
    // Parse dữ liệu vật phẩm Pawn gửi qua (data[1] chứa chuỗi JSON mảng vật phẩm)
    const items = JSON.parse(data[1]); 
    const grid = document.getElementById('inv-grid');
    grid.innerHTML = ""; // Xóa dữ liệu cũ

    const TOTAL_SLOTS = 16; // Số ô đồ mặc định là 16 (4x4)

    for (let i = 0; i < TOTAL_SLOTS; i++) {
        const slot = document.createElement("div");
        slot.classList.add("inv-slot");

        if (i < items.length) {
            // Có vật phẩm ở slot này
            slot.innerHTML = `
                <div class="item-name">${items[i].name}</div>
                <div class="item-qty">x${items[i].amount}</div>
            `;
            // Khi click vào vật phẩm -> Gửi Event về Pawn (Action: "use", ItemID)
            slot.onclick = () => {
                let outData = ["use", items[i].id];
                Cef.sendEvent("inventory_action", JSON.stringify(outData));
            };
        } else {
            // Slot trống
            slot.classList.add("empty");
        }

        grid.appendChild(slot);
    }
}

// Đăng ký event để Pawn gọi
Cef.registerEventCallback("inventory_show", "renderInventory");

/* ================= REGISTER EVENTS ================= */

Cef.registerEventCallback(
    "alert_show",
    "showAlert"
);

Cef.registerEventCallback(
    "notification_show",
    "showNotification"
);
