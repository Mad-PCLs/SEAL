#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "types/DenoiserTypes.hpp"

/* 
 * Median filter that only considers each pixel,
 * the value above and the one to the right.
 */
void three_pix_median_filter(cv::Mat &img);

/* 
 * Median filter that only considers each pixel,
 * and the values around it in a cross pattern.
 * 
 * TODO: Optimize this for memory! Using a lot of memory for intermediate results
 */
void five_pix_median_filter(cv::Mat &img);

// The standard 3x3 median filter
void three_by_three_median_filter(cv::Mat &img);

// Wrapper function to handle different DenoiserTypes
void median_filter(
    cv::Mat &img,
    DenoiserType denoiser_type
);
