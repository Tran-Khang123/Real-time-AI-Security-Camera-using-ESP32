#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

void detect_and_draw(cv::Mat &frame, cv::dnn::Net &net);