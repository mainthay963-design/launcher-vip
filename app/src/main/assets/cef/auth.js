// ====== NHẬN LỆNH TỪ PAWN SERVER ======

// Hiển thị bảng Đăng Nhập
function showLoginUI(eventData) {
    const data = JSON.parse(eventData);
    const playerName = data[0]; // Server sẽ gửi tên người chơi qua

    // Tự động điền tên và hiện bảng đăng nhập
    document.getElementById('log-username').value = playerName;
    document.getElementById('login-box').classList.remove('hidden');
    document.getElementById('auth-overlay').classList.remove('hidden');
}

// Hiển thị bảng Đăng Ký
function showRegisterUI(eventData) {
    const data = JSON.parse(eventData);
    const playerName = data[0]; // Server sẽ gửi tên người chơi qua

    // Tự động điền tên và hiện bảng đăng ký
    document.getElementById('reg-username').value = playerName;
    document.getElementById('register-box').classList.remove('hidden');
    document.getElementById('auth-overlay').classList.remove('hidden');
}

// Đăng ký Event với Plugin CEF
Cef.registerEventCallback("cef_show_login", "showLoginUI");
Cef.registerEventCallback("cef_show_register", "showRegisterUI");


// ====== GỬI DỮ LIỆU LÊN PAWN SERVER ======

// Gửi Đăng Nhập
function sendLogin() {
    const pass = document.getElementById('log-password').value;
    
    if (pass.length < 1) return; // Rào lỗi: Không cho gửi nếu trống
    
    // Gửi event "server_login_attempt" lên Pawn
    let outgoingData = [pass];
    Cef.sendEvent("server_login_attempt", JSON.stringify(outgoingData));
}

// Gửi Đăng Ký
function sendRegister() {
    const pass = document.getElementById('reg-password').value;
    const dob = document.getElementById('reg-dob').value;
    
    if (pass.length < 1 || dob.length < 1) return; // Rào lỗi
    
    // Gửi event "server_register_attempt" lên Pawn (Gồm Pass và Ngày Sinh)
    let outgoingData = [pass, dob];
    Cef.sendEvent("server_register_attempt", JSON.stringify(outgoingData));
}

// Hàm ẩn toàn bộ UI (Server Pawn sẽ gọi hàm này qua Cef.emit khi login thành công)
function hideAuthUI() {
    document.getElementById('auth-overlay').classList.add('hidden');
    document.getElementById('login-box').classList.add('hidden');
    document.getElementById('register-box').classList.add('hidden');
}
Cef.registerEventCallback("cef_hide_auth", "hideAuthUI");
