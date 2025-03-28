#include "keypoint_detection/KeypointDetector.hpp"

KeypointDetector::KeypointDetector(
    int maxKeypoints,
    double qualityLevel,
    double minDistance,
    int blockSize,
    int gradientSize,
    bool useHarrisDetector,
    double harrisK,
    GFTTCornerDerivativeType cornerDerivativeType
) :
    maxKeypoints(maxKeypoints),
    qualityLevel(qualityLevel),
    minDistance(minDistance),
    blockSize(blockSize),
    gradientSize(gradientSize),
    useHarrisDetector(useHarrisDetector),
    harrisK(harrisK),
    cornerDerivativeType(cornerDerivativeType) { }


void KeypointDetector::detect(
    cv::InputArray imageInput,
    cv::OutputArray corners,
    cv::OutputArray cornersQuality,
    cv::InputArray mask
) {
    if (imageInput.empty()) {
        corners.clear();
        cornersQuality.clear();
        return;
    }

    // Ensure image is grayscale
    cv::Mat processedImage;
    if (imageInput.isUMat()) {
        cv::UMat tempImage = imageInput.getUMat();
        if (tempImage.type() != CV_8U) {
            CV_LOG_WARNING(nullptr, "Image is colored, converting it to grayscale!");
            cv::cvtColor(tempImage, processedImage, cv::COLOR_BGR2GRAY);
        }
        else {
            processedImage = tempImage.getMat(cv::ACCESS_READ);
        }
    }
    else
    {
        cv::Mat tempImage = imageInput.getMat();
        if (tempImage.type() != CV_8U) {
            CV_LOG_WARNING(nullptr, "Image is colored, converting it to grayscale!");
            cv::cvtColor(tempImage, processedImage, cv::COLOR_BGR2GRAY);
        }
        else {
            processedImage = tempImage;
        }
    }

    // Perform keypoint detection with the processed grayscale image
    goodFeaturesToTrack(
        processedImage,
        corners,
        maxKeypoints,
        qualityLevel,
        minDistance,
        mask,
        cornersQuality,
        blockSize,
        gradientSize,
        useHarrisDetector,
        harrisK,
        cornerDerivativeType
    );

    CV_Assert(corners.size() == cornersQuality.size());
}
