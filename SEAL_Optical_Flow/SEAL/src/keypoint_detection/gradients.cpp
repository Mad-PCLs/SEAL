#include "keypoint_detection/gradients.hpp"

void binarizedDerivative(
    const cv::Mat& src,
    cv::Mat& dst,
    bool is_horizontal,
    float scaleFactor
) {
    int ddepth = CV_32F;

    // Define binarized Sobel-like kernel
    cv::Mat kernel = (is_horizontal) ? (cv::Mat_<float>(1, 3) << -1, 0, 1) \
                                     : (cv::Mat_<float>(3, 1) << -1, 0, 1);
    
    // Apply scale factor
    kernel *= scaleFactor;

    // Compute derivatives
    cv::filter2D(src, dst, ddepth, kernel);
}
