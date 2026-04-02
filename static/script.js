// --- KHỞI TẠO BIẾN ---
const canvas = document.getElementById('roiCanvas');
const ctx = canvas.getContext('2d');
const videoWrapper = document.getElementById('videoWrapper');
const connStatus = document.getElementById('connStatus');
const buzzerStatus = document.getElementById('buzzerStatus');
const galleryGrid = document.getElementById('galleryGrid');
const dateInput = document.getElementById('historyFilter');
// Biến cho AI Toggle
const aiToggle = document.getElementById('aiBuzzerToggle');
const aiStatusText = document.getElementById('aiStatusText');

let isLiveMode = true;
// Biến cờ kiểm tra đang xem Live hay xem Lịch sử cũ
let currentData = [];
// --- UPDATED: Biến theo dõi ảnh đang xem trong Modal ---
let currentImageIndex = 0;

const ACTUAL_VIDEO_WIDTH = 1280;
const ACTUAL_VIDEO_HEIGHT = 960;

// Set mặc định dateInput là hôm nay
const today = new Date().toISOString().split('T')[0];
dateInput.value = today;
// --- CẬP NHẬT TRẠNG THÁI (POLLING) ---
async function updateStatus() {
    try {
        const timestamp = new Date().getTime();
        const response = await fetch(`/api/status?t=${timestamp}`);
        if (!response.ok) throw new Error("Network error");
        const data = await response.json();
        // 1. Cập nhật Status Bar
        connStatus.innerHTML = '<i class="fas fa-wifi"></i> Đã kết nối';
        connStatus.style.color = '#2ecc71';

        // 2. Cập nhật Còi
        updateBuzzerUI(data.buzzer);
        // 3. Cập nhật AI Toggle (đồng bộ từ server)
        if (data.ai_buzzer_enabled !== undefined) {
            aiToggle.checked = data.ai_buzzer_enabled;
            updateAIText(data.ai_buzzer_enabled);
        }

        // 4. Cập nhật Gallery (Chỉ khi đang ở chế độ Live Mode)
        if (isLiveMode) {
            const latestServerId = data.history.length > 0 ?
            data.history[0].id : null;
            const latestLocalId = currentData.length > 0 ? currentData[0].id : null;
            if (latestServerId !== latestLocalId) {
                console.log("🔄 Có ảnh mới, cập nhật gallery...");
                currentData = data.history;
                renderGallery(currentData);
            }
        }

    } catch (error) {
        console.error("Lỗi cập nhật trạng thái:", error);
        connStatus.innerHTML = '<i class="fas fa-exclamation-triangle"></i> Mất kết nối';
        connStatus.style.color = '#e94560';
    }
}

function updateBuzzerUI(isOn) {
    const btnOn = document.getElementById('btnOn');
    const btnOff = document.getElementById('btnOff');
    const indicator = document.getElementById('buzzerStatus');
    if (isOn) {
        indicator.textContent = "⚠️ CẢNH BÁO: CÒI ĐANG HÚ!";
        indicator.className = "buzzer-indicator on";
        btnOn.classList.add('active');
        btnOff.classList.remove('active');
        btnOn.style.opacity = "1";
        btnOff.style.opacity = "0.5";
    } else {
        indicator.textContent = "Hệ thống an toàn";
        indicator.className = "buzzer-indicator off";
        btnOn.classList.remove('active');
        btnOff.classList.add('active');
        btnOn.style.opacity = "0.5";
        btnOff.style.opacity = "1";
    }
}

// --- LOGIC LỌC NGÀY ---
dateInput.addEventListener('change', async () => {
    const selectedDate = dateInput.value;
    const todayStr = new Date().toISOString().split('T')[0];

    galleryGrid.innerHTML = '<div style="color:white; text-align:center; padding:20px;">⏳ Đang tải dữ liệu...</div>';

    if (selectedDate === todayStr) {
        isLiveMode = true;
        const response = await fetch(`/api/status`);
        const data = await response.json();
        currentData = data.history;
   
        renderGallery(currentData);
        console.log("✅ Chuyển về chế độ LIVE");
    } else {
        isLiveMode = false;
        try {
            const response = await fetch(`/api/history?date=${selectedDate}`);
            const historyData = await response.json();
            currentData = historyData;
           
            renderGallery(currentData);
            console.log(`✅ Đã tải lịch sử ngày ${selectedDate}: ${historyData.length} ảnh`);
        } catch (e) {
            galleryGrid.innerHTML = '<div style="color:red; text-align:center;">❌ Lỗi tải dữ liệu</div>';
        }
    }
});

// --- RENDER ẢNH ---
function renderGallery(history) {
    galleryGrid.innerHTML = "";
    if (!history || history.length === 0) {
        galleryGrid.innerHTML = `
            <div style="color: #888; text-align: center; grid-column: 1/-1; padding: 20px; font-size: 0.9em;">
                <i class="far fa-images" style="font-size: 2em; margin-bottom: 10px; display: block;"></i>
                Không có dữ liệu phát hiện
            </div>
        `;
        return;
    }

    const fragment = document.createDocumentFragment();
    history.forEach((item, index) => {
        const div = document.createElement('div');
        div.className = 'gallery-item';
        div.innerHTML = `
            <img src="${item.url}" loading="lazy" alt="Detection">
            <span>${item.time}</span>
        `;
        // UPDATED: Truyền index vào thay vì chỉ url
        div.onclick = () => openModal(index);
        fragment.appendChild(div);
    });
    galleryGrid.appendChild(fragment);
}

// --- VẼ ROI (Canvas) ---
let isDrawing = false, startX, startY;
let currentVideoDisplayWidth = 0; 
let currentVideoDisplayHeight = 0;

function resizeCanvas() {
    if (!videoWrapper) return;
    const img = document.getElementById('cameraStream');
    if (!img) return;
    const imgRect = img.getBoundingClientRect();
    currentVideoDisplayWidth = imgRect.width;
    currentVideoDisplayHeight = imgRect.height;
    canvas.width = currentVideoDisplayWidth;
    canvas.height = currentVideoDisplayHeight;
}

window.addEventListener('resize', resizeCanvas);
document.addEventListener('DOMContentLoaded', () => setTimeout(resizeCanvas, 1000));
canvas.addEventListener('mousedown', e => {
    if (!currentVideoDisplayWidth || !currentVideoDisplayHeight) return; 
    isDrawing = true;
    const r = canvas.getBoundingClientRect();
    startX = e.clientX - r.left;
    startY = e.clientY - r.top;
});
canvas.addEventListener('mousemove', e => {
    if (!isDrawing || !currentVideoDisplayWidth || !currentVideoDisplayHeight) return;
    const r = canvas.getBoundingClientRect();
    const curX = e.clientX - r.left;
    const curY = e.clientY - r.top;
    drawBox(curX, curY);
});
canvas.addEventListener('mouseup', e => {
    if (!currentVideoDisplayWidth || !currentVideoDisplayHeight) return;
    const r = canvas.getBoundingClientRect();
    stopDrawing(e.clientX - r.left, e.clientY - r.top);
});
canvas.addEventListener('touchstart', e => {
    if (!currentVideoDisplayWidth || !currentVideoDisplayHeight) return;
    isDrawing = true;
    const r = canvas.getBoundingClientRect();
    const touch = e.touches[0];
    startX = touch.clientX - r.left;
    startY = touch.clientY - r.top;
    e.preventDefault();
}, { passive: false });
canvas.addEventListener('touchmove', e => {
    if (!isDrawing || !currentVideoDisplayWidth || !currentVideoDisplayHeight) return;
    const r = canvas.getBoundingClientRect();
    const touch = e.touches[0];
    const curX = touch.clientX - r.left;
    const curY = touch.clientY - r.top;
    drawBox(curX, curY);
    e.preventDefault();
}, { passive: false });
canvas.addEventListener('touchend', e => {
    if (!currentVideoDisplayWidth || !currentVideoDisplayHeight) return;
    const touch = e.changedTouches[0];
    stopDrawing(touch.clientX - r.left, touch.clientY - r.top);
});
function drawBox(curX, curY) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    const w = curX - startX;
    const h = curY - startY;
    ctx.strokeStyle = '#e94560';
    ctx.lineWidth = 3;
    ctx.strokeRect(startX, startY, w, h);
}

const stopDrawing = (endX, endY) => {
    isDrawing = false;
    const w = Math.abs(endX - startX);
    const h = Math.abs(endY - startY);
    const x = Math.min(startX, endX);
    const y = Math.min(startY, endY);
    if (w > 20 && h > 20) {
        const scaleX = ACTUAL_VIDEO_WIDTH / currentVideoDisplayWidth;
        const scaleY = ACTUAL_VIDEO_HEIGHT / currentVideoDisplayHeight;
        const roiX = Math.round(x * scaleX);
        const roiY = Math.round(y * scaleY);
        const roiW = Math.round(w * scaleX);
        const roiH = Math.round(h * scaleY);
        sendROI(roiX, roiY, roiW, roiH);
    }
};
async function sendROI(x, y, w, h) {
    try {
        const response = await fetch('/api/set_roi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ x, y, w, h }) 
        });
        if (response.ok) {
            alert("✅ Đã cập nhật vùng giám sát!");
        } else {
            throw new Error(`Server returned ${response.status}`);
        }
    } catch (error) {
        alert("❌ Lỗi: " + error.message);
    }
}

async function clearROI() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    try {
        await fetch('/api/clear_roi');
        alert("✅ Đã xóa vùng giám sát!");
    } catch (error) {
        alert("❌ Lỗi: " + error.message);
    }
}

async function manualControl(state) {
    updateBuzzerUI(state === 'on');
    try {
        await fetch(`/api/manual_buzzer/${state}`);
    } catch (error) {
        console.error("Lỗi điều khiển:", error);
    }
}

// --- TÍNH NĂNG MỚI: CHỤP ẢNH ---
async function takeSnapshot() {
    try {
        const btn = document.querySelector('.btn-info');
        const originalText = btn.innerHTML;
        btn.innerHTML = '<i class="fas fa-spinner fa-spin"></i> Đang chụp...';
        
        const response = await fetch('/api/capture_snapshot');
        const data = await response.json();
        
        btn.innerHTML = originalText;
        
        if (data.status === 'ok') {
            // Không alert làm phiền, chỉ log
            console.log("📸 Đã chụp ảnh:", data.url);
            // Cập nhật lại list ngay lập tức
            updateStatus();
        } else {
            alert("❌ Lỗi chụp ảnh: " + (data.error || "Unknown"));
        }
    } catch (e) {
        alert("❌ Lỗi kết nối");
        console.error(e);
    }
}

// --- TÍNH NĂNG MỚI: TOGGLE AI BUZZER ---
async function toggleAIMode() {
    const isChecked = aiToggle.checked;
    const state = isChecked ? 'on' : 'off';
    
    updateAIText(isChecked);
    
    try {
        await fetch(`/api/toggle_ai_buzzer/${state}`);
        console.log(`✅ Đã chuyển AI Alarm sang: ${state}`);
    } catch (e) {
        console.error("Lỗi toggle AI:", e);
        // Revert UI nếu lỗi
        aiToggle.checked = !isChecked;
        updateAIText(!isChecked);
        alert("❌ Lỗi kết nối đến server");
    }
}

function updateAIText(isChecked) {
    if (isChecked) {
        aiStatusText.textContent = "Khi phát hiện xâm nhập, còi sẽ tự động bật.";
        aiStatusText.style.color = "#888";
    } else {
        aiStatusText.textContent = "⚠️ CHÚ Ý: Còi sẽ KHÔNG kêu dù có phát hiện xâm nhập.";
        aiStatusText.style.color = "#e94560";
    }
}

// Modal
const modal = document.getElementById("imageModal");
const modalImg = document.getElementById("modalImage");

// UPDATED: Hàm mở modal theo index
function openModal(index) {
    if (index >= 0 && index < currentData.length) {
        currentImageIndex = index;
        updateModalImage();
        modal.style.display = "block";
    }
}

// UPDATED: Hàm cập nhật ảnh trong Modal
function updateModalImage() {
    if (currentData.length > 0) {
        modalImg.src = currentData[currentImageIndex].url;
    }
}

// UPDATED: Hàm chuyển đổi ảnh Next/Prev
function changeImage(n) {
    currentImageIndex += n;
    // Loop vòng tròn
    if (currentImageIndex >= currentData.length) {
        currentImageIndex = 0;
    } else if (currentImageIndex < 0) {
        currentImageIndex = currentData.length - 1;
    }
    updateModalImage();
}

function closeModal() {
    modal.style.display = "none";
}

window.onclick = e => {
    if (e.target == modal) closeModal();
};

// Cập nhật trạng thái mỗi 2 giây
setInterval(updateStatus, 2000);