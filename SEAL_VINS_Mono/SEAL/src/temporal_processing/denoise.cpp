#include "temporal_processing/denoise.hpp"

void three_pix_median_filter(cv::Mat &img) {

    // Find the 2 adjacent neighbors of each pixel
    cv::Mat right_pixels = cv::Mat::zeros(img.size(), img.type());
    cv::Mat above_pixels = cv::Mat::zeros(img.size(), img.type());

    // Copy the right-shifted pixels into the right_pixels matrix
    img.colRange(1, img.cols).copyTo(right_pixels.colRange(0, img.cols - 1));
    
    // Copy the above-shifted pixels into the above_pixels matrix
    img.rowRange(0, img.rows - 1).copyTo(above_pixels.rowRange(1, img.rows));

    // Find the median of the 3 pixels
    cv::Mat min_p1_p2 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat max_p1_p2 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat min_max = cv::Mat::zeros(img.size(), img.type());
    cv::Mat median_img = cv::Mat::zeros(img.size(), img.type());

    // let the following be the pixels, where p2 is the center pixel
    // |   | p1 |    |
    // |   | p2 | p3 |
    // p1 is above_pixels, p3 is right_pixels
    // Median = max(min(p1, p2), min(max(p1, p2), p3))
    cv::min(above_pixels, img, min_p1_p2);
    cv::max(above_pixels, img, max_p1_p2);
    cv::min(max_p1_p2, right_pixels, min_max);
    cv::max(min_p1_p2, min_max, median_img);

    img = median_img;
}

void five_pix_median_filter(cv::Mat &img) {
    // Simple median filter. Extended to sort 5 pixels.

    // Find the 4 adjacent neighbors of each pixel
    cv::Mat right_pixels = cv::Mat::zeros(img.size(), img.type());
    cv::Mat above_pixels = cv::Mat::zeros(img.size(), img.type());
    cv::Mat left_pixels = cv::Mat::zeros(img.size(), img.type());
    cv::Mat below_pixels = cv::Mat::zeros(img.size(), img.type());

    // Copy the right-shifted pixels into the right_pixels matrix
    img.colRange(1, img.cols).copyTo(right_pixels.colRange(0, img.cols - 1));
    
    // Copy the below-shifted pixels into the below_pixels matrix
    img.rowRange(1, img.rows).copyTo(below_pixels.rowRange(0, img.rows - 1));

    // Copy the left-shifted pixels into the left_pixels matrix
    img.colRange(0, img.cols - 1).copyTo(left_pixels.colRange(1, img.cols));

    // Copy the above-shifted pixels into the above_pixels matrix
    img.rowRange(0, img.rows - 1).copyTo(above_pixels.rowRange(1, img.rows));

    // Create matrices to store the intermediate sorting results
    // Ryan: Refer to 5 pixel median sorting diagram for naming reference
    cv::Mat sort_1_1 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_1_2 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_1_3 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_1_4 = cv::Mat::zeros(img.size(), img.type());

    cv::Mat sort_2_1 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_2_2 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_2_3 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_2_4 = cv::Mat::zeros(img.size(), img.type());

    cv::Mat sort_3_max = cv::Mat::zeros(img.size(), img.type());
    cv::Mat sort_3_min = cv::Mat::zeros(img.size(), img.type());

    // Last stage: 3 pixel median
    cv::Mat three_min_1 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat three_max_1 = cv::Mat::zeros(img.size(), img.type());
    cv::Mat three_min_2 = cv::Mat::zeros(img.size(), img.type());

    // Compute the median

    // Layer 1 of bitonic sorters
    cv::min(above_pixels, left_pixels, sort_1_1);
    cv::max(above_pixels, left_pixels, sort_1_2);
    cv::min(img, right_pixels, sort_1_3);
    cv::max(img, right_pixels, sort_1_4);

    // Layer 2 of bitonic sorters
    cv::min(sort_1_1, sort_1_4, sort_2_1);
    cv::max(sort_1_1, sort_1_4, sort_2_2);
    cv::min(sort_1_2, sort_1_3, sort_2_3);
    cv::max(sort_1_2, sort_1_3, sort_2_4);

    // Layer 3
    cv::max(sort_2_1, sort_2_3, sort_3_max);
    cv::min(sort_2_2, sort_2_4, sort_3_min);

    // Three Pixel Median Sort
    cv::min(sort_3_max, sort_3_min, three_min_1);
    cv::max(sort_3_max, sort_3_min, three_max_1);
    cv::min(three_max_1, below_pixels, three_min_2);

    // Final comparison
    cv::max(three_min_1, three_min_2, img);
}

void three_by_three_median_filter(cv::Mat &img) {
    cv::medianBlur(img, img, 3);
}

void median_filter(
    cv::Mat &img,
    DenoiserType denoiser_type
) {
    // If the image is color, convert it to grayscale
    if (img.channels() > 1) {
        CV_LOG_WARNING(nullptr, "Image is colored, converting it to grayscale!");
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
    }

    switch (denoiser_type)
    {
        case DenoiserType::THREE_PIX_MEDIAN:
            three_pix_median_filter(img);
            break;
        case DenoiserType::FIVE_PIX_MEDIAN:
            five_pix_median_filter(img);
            break;
        case DenoiserType::THREE_BY_THREE_MEDIAN:
            three_by_three_median_filter(img);
            break;
        default:
            std::cerr << "Invalid denoiser type!" << std::endl;
            break;
    }
}
