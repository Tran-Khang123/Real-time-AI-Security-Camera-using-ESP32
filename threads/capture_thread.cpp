#include "capture_thread.h"
#include "core/app_state.h"
#include "config/config.h"
#include <opencv2/opencv.hpp>
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdlib>

void capture_thread_func() {
    cv::VideoCapture cap;
    char const * env_val = getenv("OPENCV_FFMPEG_CAPTURE_OPTIONS");
    if(env_val == NULL) {
        #ifdef _WIN32
        _putenv("OPENCV_FFMPEG_CAPTURE_OPTIONS=rtsp_transport;tcp|fflags;discardcorrupt+nobuffer|flags;low_delay|max_delay;0|reorder_queue_size;0");
        #else
        setenv("OPENCV_FFMPEG_CAPTURE_OPTIONS",
            "rtsp_transport;tcp"
            "|fflags;discardcorrupt+nobuffer"
            "|flags;low_delay"
            "|max_delay;0"
            "|reorder_queue_size;0",
            1);
        #endif
    }
    while (is_running) {
        if (!cap.isOpened()) {
            std::cout << "Connecting to Tapo Camera..." << std::endl;
            cap.open(RTSP_URL, cv::CAP_FFMPEG);
            if (cap.isOpened()) {
                cap.set(cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_ANY);
                std::cout << "✅ Camera Connected! (GPU Enabled)" << std::endl;
            } else {
                std::this_thread::sleep_for(std::chrono::seconds(2));
                continue;
            }
        }
        cv::Mat temp;
        if (!cap.read(temp) || temp.empty()) {
            std::cout << "⚠️ Stream mat tin hieu, dang ket noi lai..." << std::endl;
            cap.release();
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(capture_mutex);
            latest_captured_frame = temp;
            has_new_frame = true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}