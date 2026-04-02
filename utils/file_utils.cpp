#include "file_utils.h"
#include "core/app_state.h"
#include "config/config.h"
#include "utils/image_utils.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <algorithm>
#include <iostream>
#include <cstdio>

namespace fs = std::filesystem;
using json = nlohmann::json;

std::vector<json> scan_images_in_folder(const fs::path& folder_path, const std::string& date_str) {
    std::vector<json> list;
    if (!fs::exists(folder_path)) return list;
    try {
        for (const auto& file : fs::directory_iterator(folder_path)) {
            if (file.path().extension() == ".jpg") {
                std::string filename = file.path().filename().string();
                auto ftime = fs::last_write_time(file);
                auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
                );
                double timestamp = (double)std::chrono::duration_cast<std::chrono::milliseconds>(sctp.time_since_epoch()).count();
                std::string time_display = "00:00:00";
                size_t underscore_pos = filename.find_last_of('_');
                if (underscore_pos != std::string::npos && filename.length() >= underscore_pos + 7) {
                    std::string time_part = filename.substr(underscore_pos + 1, 6);
                    if (time_part.length() == 6 && std::all_of(time_part.begin(), time_part.end(), ::isdigit)) {
                        time_display = time_part.substr(0, 2) + ":" + time_part.substr(2, 2) + ":" + time_part.substr(4, 2);
                    }
                }
                json item;
                item["id"] = timestamp;
                item["time"] = time_display;
                item["date_raw"] = date_str;
                item["url"] = "/captured_cats/" + date_str + "/" + filename;
                list.push_back(item);
            }
        }
        std::sort(list.begin(), list.end(), [](const json& a, const json& b) {
            return a["id"] > b["id"];
        });
    } catch (...) {}
    return list;
}

void load_todays_history() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    char date_buf[20];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm);
    std::string today_str(date_buf);
    fs::path today_dir = fs::path(SAVE_DIR) / today_str;
    std::vector<json> today_list = scan_images_in_folder(today_dir, today_str);
    detection_history.clear();
    for (const auto& item : today_list) {
        detection_history.push_back(item);
    }
    std::cout << "Da tai " << detection_history.size() << " anh cua hom nay." << std::endl;
}

void save_image_to_disk(cv::Mat frame, bool is_manual) {
    if (frame.empty()) return;
    apply_sharpening(frame);
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    char date_buf[20], time_buf[20], file_buf[100];
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d", &tm);
    strftime(time_buf, sizeof(time_buf), "%H%M%S", &tm);
    fs::path dir_path = fs::path(SAVE_DIR) / date_buf;
    fs::create_directories(dir_path);
    sprintf(file_buf, "%s_%s_%s.jpg", is_manual ? "manual" : "detect", date_buf, time_buf);
    fs::path file_path = dir_path / file_buf;
    cv::imwrite(file_path.string(), frame);
    std::cout << "Saved: " << file_path << std::endl;
    char display_time[10];
    strftime(display_time, sizeof(display_time), "%H:%M:%S", &tm);
    json entry;
    entry["id"] = (double)std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
    entry["time"] = display_time;
    entry["date_raw"] = date_buf;
    entry["url"] = "/captured_cats/" + std::string(date_buf) + "/" + std::string(file_buf);
    detection_history.insert(detection_history.begin(), entry);
    if (detection_history.size() > 300) detection_history.erase(detection_history.end() - 1);
}