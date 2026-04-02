cmake_minimum_required(VERSION 3.10)
project(Esp32CamAI LANGUAGES CXX)

# Bắt buộc C++17 cho std::filesystem và các tính năng hiện đại
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Cấu hình OpenCV (Điều chỉnh đường dẫn nếu cần)
set(OpenCV_DIR /usr/local/lib/cmake/opencv4)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "OpenCV libs: ${OpenCV_LIBS}")

# Danh sách file nguồn (theo đúng cấu trúc refactored)
set(SOURCES
    main.cpp
    core/app_state.cpp
    services/esp32_service.cpp
    services/detection_service.cpp
    threads/capture_thread.cpp
    threads/processing_thread.cpp
    utils/image_utils.cpp
    utils/file_utils.cpp
    api/routes.cpp
)

# Danh sách header (để IDE nhận diện & navigate tốt hơn)
set(HEADERS
    config/config.h
    core/app_state.h
    services/esp32_service.h
    services/detection_service.h
    threads/capture_thread.h
    threads/processing_thread.h
    utils/image_utils.h
    utils/file_utils.h
    api/routes.h
)

add_executable(Esp32Monitor ${SOURCES} ${HEADERS})

# Include directories: Root + các thư mục con + OpenCV
target_include_directories(Esp32Monitor PRIVATE
    .
    config
    core
    services
    threads
    utils
    api
    ${OpenCV_INCLUDE_DIRS}
)

# Link thư viện
target_link_libraries(Esp32Monitor PRIVATE
    ${OpenCV_LIBS}
    Threads::Threads
)

# Linux/GCC < 9.0 cần link stdc++fs riêng cho std::filesystem
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(Esp32Monitor PRIVATE stdc++fs)
endif()

# Windows specific libs
if(WIN32)
    target_link_libraries(Esp32Monitor PRIVATE ws2_32 crypt32)
endif()

# Compiler warnings (tùy chọn nhưng khuyến khích)
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(Esp32Monitor PRIVATE -Wall -Wextra -O3)
elseif(MSVC)
    target_compile_options(Esp32Monitor PRIVATE /W4 /O2)
endif()