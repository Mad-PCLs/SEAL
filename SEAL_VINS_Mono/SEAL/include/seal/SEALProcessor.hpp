#pragma once

#include <string.h>

#include <opencv2/opencv.hpp>

#include "SEALConfig.hpp"
#include "SEALConfigBuilder.hpp"
#include "temporal_processing/TemporalProcessor.hpp"
#include "keypoint_detection/KeypointDetector.hpp"
#include "keypoint_tracking/KeypointTracker.hpp"

class SEALProcessor {
    private:
        SEALConfig cfg;
        KeypointTracker tracker;
        KeypointDetector detector;
        TemporalProcessor temporal_processor;

    public:
        explicit SEALProcessor(
            const int edge_threshold
        );

        explicit SEALProcessor(
            const SEALConfig& cfg
        );

        explicit SEALProcessor(
            const std::string param_filepath
        );

        void temporal_process(cv::Mat& img);

        void detect_keypoints(
            cv::InputArray img,
            std::vector<cv::Point2f>& corners,
            std::vector<float>& cornersQuality,
            cv::InputArray mask
        );
        void detect_keypoints(
            cv::InputArray img,
            std::vector<cv::KeyPoint>& keypoints,
            cv::InputArray mask
        );

        void track_keypoints(
            cv::InputArray prevImg,
            cv::InputArray nextImg,
            cv::InputArray prevPts,
            cv::InputOutputArray nextPts,
            cv::OutputArray status,
            cv::OutputArray err = cv::noArray()
        );

        /* Helper functions:
         * They allow higher level code (i.e. VIO pipelines)
         * to decide whether they should use SEAL or run the
         * default versions.
         */
        bool isDenoiserOn();
        bool isEdgeFilterOn();
        bool isKeypointDetectorOn();
        bool isKeypointTrackerOn();

        /* Helper function that just builds the pyramid:
         * It is used in implementations that build the pyramids
         * at a different part of the code and then pass it as an argument to
         * the keypoint tracking algorithm.
         * This is done in HybVIO, as evident in its source code:
         *   - (pyramids as constant arguments) https://github.com/SpectacularAI/HybVIO/blob/main/src/tracker/optical_flow.cpp#L10
         *   - (ensureImagePyramid) https://github.com/SpectacularAI/HybVIO/blob/main/src/tracker/image.cpp#L95
         *   - (cv::buildOpticalFlowPyramid) https://github.com/SpectacularAI/HybVIO/blob/main/src/tracker/image_pyramid.cpp#L40
         * 
         * Calling this function, storing the previous and next pyramids and passing them
         * as arguments to track_keypoints(...) is equivalent to calling track_keypoints(...)
         * and providing the previous image and the next image (instead of the pyramids).
         * 
         * This function is not really needed, because if SEALProcessor::track_keypoints(...),
         * receives pyramids instead of a cv::Mat, it extracts the images needed from the
         * first level of each pyramid and only uses the images for the tracking.
         * We just provide this function for good measure.
         */
        int buildOpticalFlowPyramid(
            cv::InputArray img,
            cv::OutputArrayOfArrays pyramid
        );

};
