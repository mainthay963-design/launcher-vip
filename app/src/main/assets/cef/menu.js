const tabletMenu = document.getElementById('tabletMenu');

/* =========================================
   1. HỆ THỐNG MENU TABLET (NHẬN LỆNH TỪ GAME)
   ========================================= */

// Game gọi event này để Tắt/Mở menu. Data nhận về có thể là [1] (mở) hoặc [0] (tắt)
function toggleTabletUI(eventData) {
    const eventDataJson = JSON.parse(eventData);
    const state = parseInt(eventDataJson[0]); 

    if (state === 1) {
        tabletMenu.classList.add('show');
    } else {
        tabletMenu.classList.remove('show');
    }
}
// Đăng ký event nhận từ Pawn: Cef.registerEventCallback("tên_event_web", "tên_hàm_js")
if (typeof Cef !== 'undefined') {
    Cef.registerEventCallback("tablet_toggle", "toggleTabletUI");
}

/* =========================================
   2. HỆ THỐNG CLICK APP (GỬI LỆNH LÊN GAME)
   ========================================= */

// Hàm chạy khi click vào các icon app
function handleAppClick(appName, isLocked) {
    if (isLocked) {
        // Nếu app bị khóa -> Dùng luôn hệ thống Alert in-game
        createAlert("Hệ thống cảnh báo", `Ứng dụng "${appName}" hiện đang bị khóa hoặc bạn không có quyền!`, (response) => {
            // Callback: Người dùng bấm OK hay Cancel
            console.log(`Đã đóng thông báo khóa app: ${appName}`);
        });
    } else {
        // Nếu app không khóa -> Gửi dữ liệu về Server xử lý
        let outgoingEventData = [appName];
        
        console.log(`Đang mở app: ${appName}`);
        if (typeof Cef !== 'undefined') {
            Cef.sendEvent("tablet_app_clicked", JSON.stringify(outgoingEventData));
        } else {
            // Hiển thị tạm lúc test trên trình duyệt PC
            alert(`CEF Data Gửi Lên Máy Chủ:\nEvent: tablet_app_clicked\nData: ${appName}`);
        }
    }
}

// Bấm ra bên ngoài tablet để đóng menu (gửi lệnh về server báo là đã đóng)
document.addEventListener('click', function(event) {
    if (tabletMenu.classList.contains('show') && !tabletMenu.contains(event.target)) {
        tabletMenu.classList.remove('show');
        
        // Báo cho server Pawn biết người chơi đã tắt menu
        if (typeof Cef !== 'undefined') {
            Cef.sendEvent("tablet_closed", JSON.stringify([1]));
        }
    }
});

/* =========================================
   3. HỆ THỐNG ALERT BOX CEF (TỪ VÍ DỤ CỦA BẠN)
   ========================================= */

function createAlert(title, message, callback) {
    document.getElementById('alert-title').innerText = title;
    document.getElementById('alert-message').innerText = message;

    const overlay = document.getElementById('alert-overlay');
    overlay.classList.remove('hidden');

    document.getElementById('okay-button').onclick = () => {
        hideAlert();
        if (callback) callback(true);
    };

    document.getElementById('cancel-button').onclick = () => {
        hideAlert();
        if (callback) callback(false);
    };
}

function hideAlert() {
    const overlay = document.getElementById('alert-overlay');
    overlay.classList.add('hidden');
}

// Hàm nhận lệnh tạo hộp thoại Alert từ Pawn
function showAlert(eventData) {
    const eventDataJson = JSON.parse(eventData);
    const alertId = parseInt(eventDataJson[0]);
    const alertTitle = eventDataJson[1];
    const alertMessage = eventDataJson[2];
    
    createAlert(alertTitle, alertMessage, (response) => {
        let outgoingEventData = [alertId, response];
        
        console.log(`Response status: ${response}`);
        if (typeof Cef !== 'undefined') {
            Cef.sendEvent("alert_response", JSON.stringify(outgoingEventData));
        }
    });
}
if (typeof Cef !== 'undefined') {
    Cef.registerEventCallback("alert_show", "showAlert");
}
