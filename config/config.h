#pragma once
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

// Cấu hình Camera & ESP32
inline const std::string ESP32_IP = "192.168.2.20";
inline const std::string RTSP_URL = "rtsp://admin123:0916314484Aa@192.168.2.51:554/stream1";
inline const std::string ESP32_CMD_ON = "http://" + ESP32_IP + ":80/on";
inline const std::string ESP32_CMD_OFF = "http://" + ESP32_IP + ":80/off";
inline const std::string MODEL_PATH = "/home/rtx5070/Desktop/PROJECT_DETECT_MEO_DEPLOY_C++/runs/detect/train3/weights/best.onnx";
inline const std::string SAVE_DIR = "captured_cats";
inline constexpr int TARGET_WIDTH = 1280;
inline constexpr int TARGET_HEIGHT = 960;

// AI Constants
inline constexpr float CONFIDENCE_THRESHOLD = 0.6f;
inline constexpr float NMS_THRESHOLD = 0.45f;
inline const std::vector<std::string> CLASS_NAMES = {"CAT", "PERSON"};
inline const std::vector<cv::Scalar> COLORS = {cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255)};