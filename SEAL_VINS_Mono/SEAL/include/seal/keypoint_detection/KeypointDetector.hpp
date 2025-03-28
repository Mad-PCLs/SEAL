#pragma once

#include <opencv2/opencv.hpp>

#include "types/GFTTCornerDerivativeTypes.hpp"
#include "gftt.hpp"

class KeypointDetector {
    public:
        KeypointDetector(
            int maxKeypoints,
            double qualityLevel,
            double minDistance,
            int blockSize,
            int gradientSize,
            bool useHarrisDetector,
            double harrisK,
            GFTTCornerDerivativeType cornerDerivativeType
        );

        void detect(
            cv::InputArray imageInput,
            cv::OutputArray corners,
            cv::OutputArray cornersQuality,
            cv::InputArray mask
        );

    private:
        int maxKeypoints;
        double qualityLevel;
        double minDistance;
        int blockSize;
        int gradientSize;
        bool useHarrisDetector;
        double harrisK;
        GFTTCornerDerivativeType cornerDerivativeType;
};
