#pragma once

#include "SEALConfig.hpp"

/******************************************************************************************
 * Config builder to allow updating only some of the parameters, in an organized manner.  *
 * Useful when you only need to play around with Keypoint Detection or Keypoint Tracking. *
 ******************************************************************************************/
class SEALConfigBuilder {
private:
    SEALConfig config;

public:
    SEALConfigBuilder& setHighLevelGuards(
        bool seal_on,
        bool seal_denoiser_on,
        bool seal_edge_filter_on,
        bool seal_keypoint_detector_on,
        bool seal_lk_optical_flow_on
    );

    SEALConfigBuilder& setMedianFilterParams(DenoiserType denoiser_type);

    SEALConfigBuilder& setEdgeThreshold(int threshold);

    SEALConfigBuilder& setGFTTParams(
        GFTTCornerDerivativeType gftt_corner_derivative_type,
        int maxCorners,
        double qualityLevel,
        double minDistance,
        int blockSize,
        int gradientSize,
        bool useHarrisDetector,
        double k
    );

    SEALConfigBuilder& setOpticalFlowParams(
        LKSpatialDerivativeType lk_spatialDerivativeType,
        LKPyrDownFilterType lk_pyrDownFilterType,
        cv::Size winSize,
        int maxLevel,
        cv::TermCriteria criteria,
        int lk_flags,
        float lk_minEigThreshold
    );

    SEALConfigBuilder& loadFromYAML(
        const std::string& filename
    );

    SEALConfig build();
};
