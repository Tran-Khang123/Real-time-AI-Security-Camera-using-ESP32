# 🛡️ ESP32 AI Security Camera System
### Real-time Computer Vision • IoT Integration • Multi-threaded C++ Backend

An end-to-end **AI Surveillance System** built with **C++17, OpenCV, YOLOv8 (ONNX), and ESP32**.  
The system performs **real-time object detection from RTSP camera streams**, triggers **hardware alarms via ESP32**, and provides a **web-based monitoring dashboard**.

> 🎯 Focus: Real-time AI deployment, system architecture, multi-threading, and IoT integration (not just model training)

---

## 📸 System Overview


IP Camera (RTSP)
↓
Capture Thread (OpenCV / FFmpeg)
↓
Processing Thread (YOLOv8 ONNX - GPU/CPU)
↓
Detection + ROI Logic
↓
ESP32 Alarm (HTTP)
↓
Web Server (Live Stream + API)
↓
Browser Dashboard


---

## ⭐ Features

### 🧠 Real-time Object Detection
- YOLOv8 (ONNX) inference using OpenCV DNN
- Supports **GPU (CUDA)** and **CPU fallback**
- Detects:
  - Person
  - Cat
- Non-Maximum Suppression (NMS) for clean bounding boxes

---

### 🎯 Smart ROI Detection
- Draw Region of Interest (ROI) directly on web UI
- Alarm triggers **only when object enters ROI**
- Reduces false positives

---

### 📡 ESP32 Alarm System
- Control buzzer via HTTP request
- Heartbeat mechanism:
  - Sends "ON" every 1 second
  - Auto OFF if signal lost (fail-safe)
- Supports:
  - Auto mode (AI controlled)
  - Manual mode (user controlled)

---

### ⚙️ Multi-threaded Architecture
| Thread | Responsibility |
|-------|----------------|
| Capture Thread | Read RTSP stream |
| Processing Thread | Run AI inference |
| Main Thread | Web server |

→ Ensures **low latency** and **non-blocking processing**

---

### 🌐 Web Dashboard
Access: `http://localhost:8000`

Features:
- Live video streaming
- ROI drawing
- Toggle AI alarm
- Manual buzzer control
- Detection history
- Snapshot capture

---

### 📁 Evidence Snapshot
When intrusion detected:

captured_cats/YYYY-MM-DD/detect_YYYY-MM-DD_HHMMSS.jpg


- Stored automatically
- Viewable in web UI

---

## 🏗️ Project Structure


project/
├── main.cpp # Entry point
├── CMakeLists.txt
├── config/
│ └── config.h # Constants (RTSP, ESP32, model path)
├── core/
│ ├── app_state.h # Global state (extern)
│ └── app_state.cpp
├── services/
│ ├── detection_service.* # YOLO inference
│ └── esp32_service.* # ESP32 communication
├── threads/
│ ├── capture_thread.* # RTSP capture
│ └── processing_thread.* # AI processing
├── utils/
│ ├── image_utils.* # Resize, sharpen
│ └── file_utils.* # Save image, history
├── api/
│ ├── routes.h
│ └── routes.cpp # REST API
├── templates/
│ └── index.html # Web UI
├── static/
└── captured_cats/


---

## 🧰 Tech Stack

| Category | Technology |
|---------|------------|
| Language | C++17 |
| AI | YOLOv8 (ONNX) |
| Computer Vision | OpenCV DNN |
| Web Server | cpp-httplib |
| JSON | nlohmann/json |
| Streaming | RTSP |
| Hardware | ESP32 |
| Build | CMake |
| Threading | std::thread, mutex, atomic |

---

## ⚙️ Setup & Run

### 1. Install Dependencies (Ubuntu)
```bash
sudo apt update
sudo apt install build-essential cmake libopencv-dev
2. Build Project
mkdir build
cd build
cmake ..
make
3. Run
./Esp32Monitor

Open browser:

http://localhost:8000
⚙️ Configuration

Edit file:

config/config.h

Set:

RTSP_URL   // Camera stream
ESP32_IP   // ESP32 address
MODEL_PATH // YOLOv8 ONNX path
🧠 System Workflow
Capture thread reads RTSP stream
Processing thread runs YOLOv8 inference
If object detected inside ROI:
Trigger alarm
Send HTTP request to ESP32
Save snapshot
Web server streams video & handles API
User controls system via browser
🎯 Project Highlights

This project demonstrates:

Real-time AI inference in C++
Multi-threaded system design
RTSP video streaming
GPU acceleration (CUDA)
REST API backend (C++)
IoT integration (ESP32)
Full pipeline: AI → Backend → Hardware → UI
👨‍💻 Author

Tran Khang
AI / Computer Vision

GitHub: https://github.com/Tran-Khang123