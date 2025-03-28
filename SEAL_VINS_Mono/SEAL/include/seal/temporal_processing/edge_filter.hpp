#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#define EDGE_PIXEL_VALUE 255

// A wider edge filter (1D kernels: [-1 0 1], [-1; 0; 1])
void rl_fast_edge_filter_wide(
    cv::Mat &img,
    int edge_threshold
);
