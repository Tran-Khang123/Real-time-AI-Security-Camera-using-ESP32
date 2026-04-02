#include "image_utils.h"
#include "config/config.h"

void quick_resize_frame(cv::Mat &image) {
    if (image.empty()) return;
    cv::resize(image, image, cv::Size(TARGET_WIDTH, TARGET_HEIGHT), 0, 0, cv::INTER_LINEAR);
}

void apply_sharpening(cv::Mat &image) {
    if (image.empty()) return;
    cv::Mat gaussian;
    cv::GaussianBlur(image, gaussian, cv::Size(0, 0), 3.0);
    cv::addWeighted(image, 1.5, gaussian, -0.5, 0, image);
}