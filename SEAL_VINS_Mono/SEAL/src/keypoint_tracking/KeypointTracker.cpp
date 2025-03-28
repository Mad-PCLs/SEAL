#include "keypoint_tracking/KeypointTracker.hpp"

KeypointTracker::KeypointTracker(
    cv::Size winSize,
    int maxLevel,
    cv::TermCriteria criteria,
    int flags,
    double minEigThreshold,
    LKSpatialDerivativeType spatialDerivativeType,
    LKPyrDownFilterType pyrDownFilterType
) : winSize(winSize), maxLevel(maxLevel), criteria(criteria), flags(flags),
    minEigThreshold(minEigThreshold), spatialDerivativeType(spatialDerivativeType),
    pyrDownFilterType(pyrDownFilterType) {}

// Same as in: https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L1432
// we create a new instance of the SparsePyrLKOpticalFlowSealImpl class, do the calculations and discard.
void KeypointTracker::track(
    cv::InputArray prevImg,
    cv::InputArray nextImg,
    cv::InputArray prevPts,
    cv::InputOutputArray nextPts,
    cv::OutputArray status,
    cv::OutputArray err
) {
    SparsePyrLKOpticalFlowSealImpl optical_flow(
        winSize,
        maxLevel,
        criteria,
        flags,
        minEigThreshold,
        spatialDerivativeType,
        pyrDownFilterType
    );

    optical_flow.calc(
        prevImg,
        nextImg,
        prevPts,
        nextPts,
        status,
        err
    );
}

int KeypointTracker::buildOpticalFlowPyramid(
    cv::InputArray img,
    cv::OutputArrayOfArrays pyramid,
    bool withDerivatives,
    int pyrBorder,
    int derivBorder,
    bool tryReuseInputImage
) {
    SparsePyrLKOpticalFlowSealImpl optical_flow(
        winSize,
        maxLevel,
        criteria,
        flags,
        minEigThreshold,
        spatialDerivativeType,
        pyrDownFilterType
    );

    return optical_flow.buildOpticalFlowPyramid(
        img,
        pyramid,
        withDerivatives,
        pyrBorder,
        derivBorder,
        tryReuseInputImage
    );
}
