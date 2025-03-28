#pragma once

#include <opencv2/opencv.hpp>

#include "types/DenoiserTypes.hpp"
#include "temporal_processing/denoise.hpp"
#include "temporal_processing/edge_filter.hpp"

class TemporalProcessor {
    public:
        TemporalProcessor(
            DenoiserType denoiser_type,
            int edge_threshold
        );

        void denoise(cv::Mat& img);
        void edge_filter(cv::Mat& img);

    private:
        DenoiserType denoiser_type;
        int edge_threshold;
};
