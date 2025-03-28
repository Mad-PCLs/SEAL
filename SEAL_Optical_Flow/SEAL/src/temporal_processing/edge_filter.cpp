#include "temporal_processing/edge_filter.hpp"

// TODO: PCL: Change input type of cv::Mat to cv::InputArray for wider compatibility with different types of input images
void rl_fast_edge_filter_wide(
    cv::Mat &img,
    int edge_threshold
) {
    // Ensure the image is in grayscale
    if (img.channels() > 1) {
        CV_LOG_WARNING(nullptr, "Image is colored, converting it to grayscale!");
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }

    // Define kernels for horizontal and vertical edge detection
    cv::Mat kernel_x = (cv::Mat_<float>(1, 3) << -1, 0, 1);
    cv::Mat kernel_y = (cv::Mat_<float>(3, 1) << -1, 0, 1);

    // Apply convolution to compute horizontal and vertical differences
    cv::Mat diff_x, diff_y;
    cv::filter2D(img, diff_x, CV_32F, kernel_x); // Horizontal edges
    cv::filter2D(img, diff_y, CV_32F, kernel_y); // Vertical edges

    // Convert to absolute values
    diff_x = cv::abs(diff_x);
    diff_y = cv::abs(diff_y);

    // Create a mask based on the threshold
    cv::Mat mask = (diff_x >= edge_threshold) | (diff_y >= edge_threshold);

    // Set pixels to edge_pixel_value where the mask is true, otherwise set to 0
    img.setTo(EDGE_PIXEL_VALUE, mask);
    img.setTo(0, ~mask);
}
