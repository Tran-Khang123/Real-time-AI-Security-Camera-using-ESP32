#include "esp32_service.h"
#include "httplib.h"
#include <thread>
#include "config/config.h"

void send_esp32_command(const std::string& type) {
    std::thread([type]() {
        try {
            httplib::Client cli(ESP32_IP, 80);
            cli.set_connection_timeout(1);
            if (type == "on") cli.Get("/on");
            else cli.Get("/off");
        } catch (...) {}
    }).detach();
}