#pragma once

#include <opencv2/opencv.hpp>

#include "keypoint_tracking/SparsePyrLKOpticalFlowSealImpl.hpp"

// Comments on design choices:
// In our use case, it would be prefferable to use a simple function instead of a class for the optical flow.
// In the spirit of preserving the same code structure with OpenCV,
// we keep the code in SparsePyrLKOpticalFlowSealImpl similar to SparsePyrLKOpticalFlowImpl
// and wrap it in our own class to simplify the interface.

// Q1: Why not define and use cv::calcOpticalFlowPyrLK as a function as they do in OpenCV
//     (https://github.com/opencv/opencv/blob/4.x/modules/video/src/lkpyramid.cpp#L1432)?
// The main reason is that it allows us to de-clutter the code in SparsePyrLKOpticalFlowSealImpl.

// Q2: Why wrap SparsePyrLKOpticalFlowSealImpl inside another class, instead of a function?
// We like preserving variables that should not change (dynamically) between frames, inside a class.
// This way, we can only call each methd using cv::InputArray(s) and cv::OutputArray(s) and simplify
// the interface. (i.e. the user doesn't have to pass each time variables that remain constant).

class KeypointTracker {
    public:
        KeypointTracker(
            cv::Size winSize,
            int maxLevel,
            cv::TermCriteria criteria,
            int flags,
            double minEigThreshold,
            LKSpatialDerivativeType spatialDerivativeType,
            LKPyrDownFilterType pyrDownFilterType,
            bool useInitialFlow
        );

        void track(
            cv::InputArray prevImg,
            cv::InputArray nextImg,
            cv::InputArray prevPts,
            cv::InputOutputArray nextPts,
            cv::OutputArray status,
            cv::OutputArray err
        );

        int buildOpticalFlowPyramid(
            cv::InputArray img,
            cv::OutputArrayOfArrays pyramid,
            bool withDerivatives = true,
            int pyrBorder = cv::BORDER_REFLECT_101,
            int derivBorder = cv::BORDER_CONSTANT,
            bool tryReuseInputImage = true
        );

    private:
        cv::Size winSize;
        int maxLevel;
        cv::TermCriteria criteria;
        int flags;
        double minEigThreshold;
        LKSpatialDerivativeType spatialDerivativeType;
        LKPyrDownFilterType pyrDownFilterType;
        bool useInitialFlow;
};
