#pragma once
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include "json.hpp"

std::vector<nlohmann::json> scan_images_in_folder(const std::filesystem::path& folder_path, const std::string& date_str);
void load_todays_history();
void save_image_to_disk(cv::Mat frame, bool is_manual);