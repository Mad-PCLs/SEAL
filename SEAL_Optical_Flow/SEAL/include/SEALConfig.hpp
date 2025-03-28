#pragma once

#include <iostream>

#include <opencv2/opencv.hpp>

#include "types/DenoiserTypes.hpp"
#include "types/GFTTCornerDerivativeTypes.hpp"
#include "types/LKSpatialDerivativeTypes.hpp"
#include "types/LKPyrDownFilterTypes.hpp"

/*********************************
 * Default values for SEALConfig *
 *********************************/
struct SEALConfig {
    // High level guards
    bool seal_on = true;
    bool seal_denoiser_on = true;
    bool seal_edge_filter_on = true;
    bool seal_keypoint_detector_on = true;
    bool seal_lk_optical_flow_on = true;

    // Median filter params
    DenoiserType denoiser_type = DenoiserType::THREE_PIX_MEDIAN;

    // Edge filter params
    int edge_threshold = 17;

    // Keypoint Detection params
    GFTTCornerDerivativeType gftt_corner_derivative_type = GFTTCornerDerivativeType::BINARIZED;
    int gftt_maxCorners = 1000;
    double gftt_qualityLevel = 0.01;
    double gftt_minDistance = 1;
    int gftt_blockSize = 3;
    int gftt_gradientSize = 3;
    bool gftt_useHarrisDetector = false;
    double gftt_k = 0.04;

    // LK optical flow parameters
    LKSpatialDerivativeType lk_spatialDerivativeType = LKSpatialDerivativeType::BINARIZED;
    LKPyrDownFilterType lk_pyrDownFilterType = LKPyrDownFilterType::BOX_2x2;
    cv::Size lk_winSize = cv::Size(21, 21);
    int lk_maxLevel = 3;
    cv::TermCriteria lk_termCriteria = cv::TermCriteria(
        cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01
    );
    int lk_flags = 0;
    float lk_minEigThreshold = 1e-4;
    bool lk_useInitialFlow = true;

    // Default constructor
    SEALConfig() = default;

    // Print function
    friend std::ostream& operator<<(std::ostream& os, const SEALConfig& config) {
        os << "******** SEAL Configuration ********\n";
        os << "seal_on: " << std::boolalpha << config.seal_on << "\n";
        os << "  seal_denoiser_on: " << config.seal_denoiser_on << "\n";
        os << "  seal_edge_filter_on: " << config.seal_edge_filter_on << "\n";
        os << "  seal_keypoint_detector_on: " << config.seal_keypoint_detector_on << "\n";
        os << "  seal_lk_optical_flow_on: " << config.seal_lk_optical_flow_on << "\n\n";

        os << "--- Denoiser Parameters ---\n";
        os << "denoiser_type: " << getDenoiserTypeAsString(config.denoiser_type) << "\n\n";

        os << "--- Edge Filter Parameters ---\n";
        os << "edge_threshold: " << config.edge_threshold << "\n\n";

        os << "--- Keypoint Detection Parameters ---\n";
        os << "gftt_corner_derivative_type: " 
           << getGFTTCornerDerivativeTypeAsString(config.gftt_corner_derivative_type) << "\n";
        os << "gftt_maxCorners: " << config.gftt_maxCorners << "\n";
        os << "gftt_qualityLevel: " << config.gftt_qualityLevel << "\n";
        os << "gftt_minDistance: " << config.gftt_minDistance << "\n";
        os << "gftt_blockSize: " << config.gftt_blockSize << "\n";
        os << "gftt_gradientSize: " << config.gftt_gradientSize << "\n";
        os << "gftt_useHarrisDetector: " << config.gftt_useHarrisDetector << "\n";
        os << "gftt_k: " << config.gftt_k << "\n\n";

        os << "--- LK Optical Flow Parameters ---\n";
        os << "lk_spatialDerivativeType: " 
           << getLKSpatialDerivativeTypeAsString(config.lk_spatialDerivativeType) << "\n";
        os << "lk_pyrDownFilterType: " 
           << getLKPyrDownFilterTypeAsString(config.lk_pyrDownFilterType) << "\n";
        os << "lk_winSize: (" << config.lk_winSize.width << ", " << config.lk_winSize.height << ")\n";
        os << "lk_maxLevel: " << config.lk_maxLevel << "\n";
        os << "lk_termCriteria: ("
           << "COUNT=" << config.lk_termCriteria.maxCount << ", "
           << "EPS=" << config.lk_termCriteria.epsilon << ")\n";
         os << "lk_flags: " << config.lk_flags << "\n";
         os << "lk_minEigThreshold: " << config.lk_minEigThreshold << "\n";
         os << "lk_useInitialFlow: " << config.lk_useInitialFlow << "\n";

        os << "*************************************\n";
        return os;
    }
};
