#include "processing_thread.h"
#include "core/app_state.h"
#include "config/config.h"
#include "services/detection_service.h"
#include "utils/image_utils.h"
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <thread>
#include <chrono>
#include <iostream>

void processing_thread_func() {
    cv::dnn::Net net;
    try {
        net = cv::dnn::readNetFromONNX(MODEL_PATH);
        std::cout << "🚀 Kich hoat CUDA (GPU)..." << std::endl;
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);
    } catch (...) {
        std::cout << "❌ Loi CUDA, chay CPU..." << std::endl;
        net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    }
    while (is_running) {
        cv::Mat frame;
        bool process_now = false;
        if (has_new_frame) {
            std::lock_guard<std::mutex> lock(capture_mutex);
            if (!latest_captured_frame.empty()) {
                frame = latest_captured_frame.clone();
                has_new_frame = false;
                process_now = true;
            }
        }
        if (process_now) {
            quick_resize_frame(frame);
            {
                std::lock_guard<std::mutex> lock(display_mutex);
                raw_frame = frame.clone();
            }
            detect_and_draw(frame, net);
            {
                std::lock_guard<std::mutex> lock(display_mutex);
                output_frame = frame;
            }
        } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    }
}