#include <iostream>
#include <filesystem>
#include <thread>
#include "httplib.h"
#include "config/config.h"
#include "core/app_state.h"
#include "utils/file_utils.h"
#include "threads/capture_thread.h"
#include "threads/processing_thread.h"
#include "api/routes.h"

int main() {
    // 1. Khởi tạo
    std::filesystem::create_directories(SAVE_DIR);
    load_todays_history();

    // Khởi chạy threads
    std::thread t_capture(capture_thread_func);
    std::thread t_process(processing_thread_func);
    t_capture.detach();
    t_process.detach();

    httplib::Server svr;
    // Đăng ký tất cả routes
    setup_routes(svr);

    std::cout << "SERVER STARTED AT http://0.0.0.0:8000" << std::endl;
    svr.listen("0.0.0.0", 8000);

    is_running = false;
    return 0;
}