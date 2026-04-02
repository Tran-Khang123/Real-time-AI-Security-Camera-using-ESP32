#include "detection_service.h"
#include "core/app_state.h"
#include "config/config.h"
#include "services/esp32_service.h"
#include "utils/file_utils.h"
#include <iostream>
#include <chrono>

void detect_and_draw(cv::Mat &frame, cv::dnn::Net &net) {
    if (frame.empty()) return;
    try {
        cv::Mat blob;
        cv::dnn::blobFromImage(frame, blob, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false);
        net.setInput(blob);
        std::vector<cv::Mat> outputs;
        net.forward(outputs, net.getUnconnectedOutLayersNames());
        if (outputs.empty()) return;
        cv::Mat output0 = outputs[0];
        int dimensions = output0.size[1];
        int rows = output0.size[2];
        cv::Mat output_reshaped = output0.reshape(1, dimensions);
        cv::Mat output_transposed;
        cv::transpose(output_reshaped, output_transposed);
        if (!output_transposed.isContinuous()) {
            output_transposed = output_transposed.clone();
        }
        float *data = (float *)output_transposed.data;
        float x_factor = frame.cols / 640.0;
        float y_factor = frame.rows / 640.0;
        std::vector<int> class_ids;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;
        bool object_detected_in_roi = false;

        for (int i = 0; i < rows; ++i) {
            float *row_ptr = data + (i * dimensions);
            float max_score = 0.0;
            int max_class_id = -1;
            if (row_ptr[4] > row_ptr[5]) {
                max_score = row_ptr[4];
                max_class_id = 0;
            } else {
                max_score = row_ptr[5];
                max_class_id = 1;
            }
            if (max_score >= CONFIDENCE_THRESHOLD) {
                float cx = row_ptr[0];
                float cy = row_ptr[1];
                float w = row_ptr[2];
                float h = row_ptr[3];
                int left = int((cx - 0.5 * w) * x_factor);
                int top = int((cy - 0.5 * h) * y_factor);
                int width = int(w * x_factor);
                int height = int(h * y_factor);
                boxes.push_back(cv::Rect(left, top, width, height));
                confidences.push_back(max_score);
                class_ids.push_back(max_class_id);
            }
        }

        std::vector<int> nms_result;
        cv::dnn::NMSBoxes(boxes, confidences, CONFIDENCE_THRESHOLD, NMS_THRESHOLD, nms_result);

        for (int idx : nms_result) {
            cv::Rect box = boxes[idx];
            int cls_id = class_ids[idx];
            float conf = confidences[idx];
            if (cls_id < 0 || cls_id >= CLASS_NAMES.size()) continue;
            box.x = std::max(0, box.x);
            box.y = std::max(0, box.y);
            if (box.x + box.width > frame.cols) box.width = frame.cols - box.x;
            if (box.y + box.height > frame.rows) box.height = frame.rows - box.y;
            if (box.width <= 0 || box.height <= 0) continue;
            cv::Scalar color = COLORS[cls_id];
            cv::rectangle(frame, box, color, 3);

            int cx = box.x + box.width / 2;
            int cy = box.y + box.height / 2;
            bool in_roi = true;
            if (state.roi.active) {
                if (cx < state.roi.x || cx > state.roi.x + state.roi.w ||
                    cy < state.roi.y || cy > state.roi.y + state.roi.h) {
                    in_roi = false;
                }
            }
            if (in_roi) object_detected_in_roi = true;

            std::string label = CLASS_NAMES[cls_id] + ": " + std::to_string((int)(conf * 100)) + "%";
            cv::Size labelSize = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, nullptr);
            int label_y = std::max(0, box.y - 10);
            if (box.y < labelSize.height + 10) label_y = box.y + labelSize.height + 10;
            cv::Rect bgRect(box.x, label_y - labelSize.height, labelSize.width, labelSize.height + 5);
            if (bgRect.x + bgRect.width > frame.cols) bgRect.x = frame.cols - bgRect.width;
            if (bgRect.y < 0) bgRect.y = 0;
            cv::rectangle(frame, bgRect, color, -1);
            cv::putText(frame, label, cv::Point(bgRect.x, bgRect.y + labelSize.height - 2), cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255,255,255), 2);
        }

        if (state.roi.active) {
            cv::Scalar roi_color = object_detected_in_roi ? cv::Scalar(0,0,255) : cv::Scalar(255,0,0);
            cv::rectangle(frame, cv::Rect(state.roi.x, state.roi.y, state.roi.w, state.roi.h), roi_color, 2);
            cv::putText(frame, "ZONE", cv::Point(state.roi.x, state.roi.y - 10), cv::FONT_HERSHEY_SIMPLEX, 0.8, roi_color, 2);
        }

        double now_sec = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::system_clock::now().time_since_epoch()).count();
        if (state.is_manual_mode) {
            if (state.is_buzzer_active) {
                if (now_sec - state.last_heartbeat_sent > 1.0) {
                    send_esp32_command("on");
                    state.last_heartbeat_sent = now_sec;
                }
            }
        } else {
            if (object_detected_in_roi && state.is_ai_buzzer_enabled) {
                if (!state.is_buzzer_active) {
                    std::cout << "🚨 INTRUDER DETECTED - ALARM ON" << std::endl;
                    state.is_buzzer_active = true;
                }
                if (now_sec - state.last_heartbeat_sent > 1.0) {
                    send_esp32_command("on");
                    state.last_heartbeat_sent = now_sec;
                }
                if (now_sec - state.last_alert_time > 5) {
                    cv::Mat save_frame;
                    {
                        std::lock_guard<std::mutex> lock(display_mutex);
                        if (!raw_frame.empty()) save_frame = raw_frame.clone();
                    }
                    if (!save_frame.empty()) {
                        save_image_to_disk(save_frame, false);
                        state.last_alert_time = now_sec;
                    }
                }
            } else {
                if (state.is_buzzer_active) {
                    send_esp32_command("off");
                    state.is_buzzer_active = false;
                    std::cout << "✅ ALARM OFF (Object left ROI)" << std::endl;
                }
            }
        }
    } catch (const cv::Exception& e) {
        std::cerr << "⚠️ OpenCV Error: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "⚠️ Unknown Error" << std::endl;
    }
}