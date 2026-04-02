#pragma once
#include <mutex>
#include <atomic>
#include <opencv2/opencv.hpp>
#include "json.hpp"

struct AppState {
    bool is_buzzer_active = false;
    bool is_manual_mode = false;
    bool is_ai_buzzer_enabled = true;
    double last_alert_time = 0;
    double last_heartbeat_sent = 0;
    struct ROI {
        int x = 0, y = 0, w = 0, h = 0;
        bool active = false;
    } roi;
};

// Thread Safety & Global State
extern cv::Mat latest_captured_frame;
extern cv::Mat output_frame;
extern cv::Mat raw_frame;
extern std::mutex capture_mutex;
extern std::mutex display_mutex;
extern std::atomic<bool> is_running;
extern std::atomic<bool> has_new_frame;
extern AppState state;
extern nlohmann::json detection_history;