#include "routes.h"
#include "core/app_state.h"
#include "config/config.h"
#include "utils/file_utils.h"
#include "services/esp32_service.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;
using json = nlohmann::json;

void setup_routes(httplib::Server &svr) {
    // Static Files
    svr.set_mount_point("/captured_cats", "./captured_cats");
    svr.set_mount_point("/static", "./static");

    // Web Interface
    svr.Get("/", [](const httplib::Request &, httplib::Response &res) {
        std::ifstream t("./templates/index.html");
        if (t.is_open()) {
            std::stringstream buffer;
            buffer << t.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.set_content("<h1>Error: Missing templates/index.html</h1>", "text/html");
        }
    });

    // Video Stream
    svr.Get("/video_feed", [](const httplib::Request &, httplib::Response &res) {
        res.set_content_provider("multipart/x-mixed-replace; boundary=frame", [&](size_t offset, httplib::DataSink &sink) {
            while (true) {
                std::vector<uchar> buffer;
                bool ok = false;
                {
                    std::lock_guard<std::mutex> lock(display_mutex);
                    if (!output_frame.empty()) {
                        std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 80};
                        cv::imencode(".jpg", output_frame, buffer, params);
                        ok = true;
                    }
                }
                if (ok) {
                    std::string head = "--frame\r\nContent-Type: image/jpeg\r\nContent-Length: " + std::to_string(buffer.size()) + "\r\n\r\n";
                    sink.write(head.c_str(), head.size());
                    sink.write((const char*)buffer.data(), buffer.size());
                    sink.write("\r\n", 2);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
                if (!sink.is_writable()) break;
            }
            return true;
        });
    });

    // API Status
    svr.Get("/api/status", [](const httplib::Request &, httplib::Response &res) {
        json j;
        j["buzzer"] = state.is_buzzer_active;
        j["ai_buzzer_enabled"] = state.is_ai_buzzer_enabled;
        j["history"] = detection_history;
        res.set_content(j.dump(), "application/json");
    });

    // API History
    svr.Get("/api/history", [](const httplib::Request &req, httplib::Response &res) {
        if (req.has_param("date")) {
            std::string date_str = req.get_param_value("date");
            fs::path target_dir = fs::path(SAVE_DIR) / date_str;
            std::vector<json> result_list = scan_images_in_folder(target_dir, date_str);
            json j = result_list;
            res.set_content(j.dump(), "application/json");
        } else {
            res.set_content("[]", "application/json");
        }
    });

    // API Controls
    svr.Get(R"(/api/manual_buzzer/(.*))", [](const httplib::Request &req, httplib::Response &res) {
        std::string s = req.matches[1];
        state.is_manual_mode = (s == "on");
        state.is_buzzer_active = (s == "on");
        send_esp32_command(s);
        res.set_content("{\"status\":\"executed\"}", "application/json");
    });

    svr.Get(R"(/api/toggle_ai_buzzer/(.*))", [](const httplib::Request &req, httplib::Response &res) {
        std::string s = req.matches[1];
        state.is_ai_buzzer_enabled = (s == "on");
        res.set_content("{\"status\":\"ok\"}", "application/json");
    });

    // Capture Snapshot
    svr.Get("/api/capture_snapshot", [](const httplib::Request &, httplib::Response &res) {
        cv::Mat frame;
        {
            std::lock_guard<std::mutex> lock(display_mutex);
            if (!raw_frame.empty()) frame = raw_frame.clone();
        }
        if (!frame.empty()) {
            save_image_to_disk(frame, true);
            std::string last_url = "";
            if (!detection_history.empty()) last_url = detection_history[0]["url"];
            json resp;
            resp["status"] = "ok";
            resp["url"] = last_url;
            res.set_content(resp.dump(), "application/json");
        } else {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    // ROI APIs
    svr.Post("/api/set_roi", [](const httplib::Request &req, httplib::Response &res) {
        try {
            auto j = json::parse(req.body);
            state.roi.x = j["x"]; state.roi.y = j["y"];
            state.roi.w = j["w"]; state.roi.h = j["h"];
            state.roi.active = true;
            res.set_content("{\"status\":\"ok\"}", "application/json");
        } catch (...) {
            res.set_content("{\"status\":\"error\"}", "application/json");
        }
    });

    svr.Get("/api/clear_roi", [](const httplib::Request &, httplib::Response &res) {
        state.roi.active = false;
        res.set_content("{\"status\":\"cleared\"}", "application/json");
    });
}