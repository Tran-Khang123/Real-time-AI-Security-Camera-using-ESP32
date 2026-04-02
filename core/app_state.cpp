#include "app_state.h"

cv::Mat latest_captured_frame;
cv::Mat output_frame;
cv::Mat raw_frame;
std::mutex capture_mutex;
std::mutex display_mutex;
std::atomic<bool> is_running(true);
std::atomic<bool> has_new_frame(false);
AppState state;
nlohmann::json detection_history = nlohmann::json::array();