# 🛡️ ESP32 AI Security Camera System

### Real-time Computer Vision • IoT Integration • Multi-threaded C++ Backend

An end-to-end **AI Surveillance System** built with **C++17, OpenCV, YOLOv8 (ONNX), and ESP32**.
The system performs **real-time object detection from RTSP camera streams**, triggers **hardware alarms via ESP32**, and provides a **web-based monitoring dashboard**.

> 🎯 Focus: Real-time AI deployment, system architecture, multi-threading, and IoT integration

---

## 📸 System Overview

```
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
```

---

## ⭐ Features

### 🧠 Real-time Object Detection

* YOLOv8 (ONNX) inference using OpenCV DNN
* Supports GPU (CUDA) and CPU fallback
* Detects Person, Cat
* Uses Non-Maximum Suppression (NMS)

---

### 🎯 Smart ROI Detection

* Draw ROI directly on web UI
* Alarm triggers only when object enters ROI
* Reduces false positives

---

### 📡 ESP32 Alarm System

* Control buzzer via HTTP request
* Heartbeat mechanism:

  * Send "ON" every 1s
  * Auto OFF if lost signal
* Supports Auto mode & Manual mode

---

### ⚙️ Multi-threaded Architecture

| Thread            | Responsibility   |
| ----------------- | ---------------- |
| Capture Thread    | Read RTSP stream |
| Processing Thread | Run AI inference |
| Main Thread       | Web server       |

→ Low latency, non-blocking system

---

### 🌐 Web Dashboard

Access: `http://localhost:8000`

* Live video streaming
* ROI drawing
* Toggle AI alarm
* Manual buzzer control
* Detection history
* Snapshot capture

---

### 📁 Evidence Snapshot

```
captured_cats/YYYY-MM-DD/detect_YYYY-MM-DD_HHMMSS.jpg
```

---

## 🏗️ Project Structure

```
project/
├── main.cpp
├── CMakeLists.txt
├── config/
│   └── config.h
├── core/
│   ├── app_state.h
│   └── app_state.cpp
├── services/
│   ├── detection_service.*
│   └── esp32_service.*
├── threads/
│   ├── capture_thread.*
│   └── processing_thread.*
├── utils/
│   ├── image_utils.*
│   └── file_utils.*
├── api/
│   ├── routes.h
│   └── routes.cpp
├── templates/
│   └── index.html
├── static/
└── captured_cats/
```

---

## 🧰 Tech Stack

* C++17
* OpenCV 4.x
* YOLOv8 (ONNX)
* cpp-httplib
* nlohmann/json
* ESP32
* RTSP Streaming
* CMake

---

## ⚙️ Setup & Run

### Install dependencies

```bash
sudo apt update
sudo apt install build-essential cmake libopencv-dev
```

### Build

```bash
mkdir build
cd build
cmake ..
make
```

### Run

```bash
./Esp32Monitor
```

Open browser:

```
http://localhost:8000
```

---

## ⚙️ Configuration

Edit file:

```
config/config.h
```

Set:

* RTSP_URL
* ESP32_IP
* MODEL_PATH

---

## 🧠 System Workflow

1. Capture thread reads RTSP stream
2. Processing thread runs YOLOv8
3. Detect object inside ROI → trigger alarm
4. Send HTTP to ESP32
5. Save snapshot
6. Stream video to web UI

---

## 🎯 Project Highlights

* Real-time AI inference in C++
* Multi-threaded system design
* RTSP video streaming
* GPU acceleration (CUDA)
* REST API backend
* IoT integration (ESP32)
* End-to-end AI system

---

## 👨‍💻 Author

**Tran Khang**
AI / Computer Vision

GitHub: https://github.com/Tran-Khang123
