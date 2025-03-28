#pragma once

#include <opencv2/opencv.hpp>

void binarizedDerivative(
    const cv::Mat& src,
    cv::Mat& dst,
    bool is_horizontal,
    float scaleFactor
);
