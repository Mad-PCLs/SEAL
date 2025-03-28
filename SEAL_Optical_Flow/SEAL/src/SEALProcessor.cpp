#include "SEALProcessor.hpp"

SEALProcessor::SEALProcessor(const SEALConfig& cfg) : 
    cfg(cfg), 
    tracker(
        cfg.lk_winSize,
        cfg.lk_maxLevel,
        cfg.lk_termCriteria,
        cfg.lk_flags,
        cfg.lk_minEigThreshold,
        cfg.lk_spatialDerivativeType,
        cfg.lk_pyrDownFilterType,
        cfg.lk_useInitialFlow
    ),
    detector(
        cfg.gftt_maxCorners,
        cfg.gftt_qualityLevel,
        cfg.gftt_minDistance,
        cfg.gftt_blockSize,
        cfg.gftt_gradientSize,
        cfg.gftt_useHarrisDetector,
        cfg.gftt_k,
        cfg.gftt_corner_derivative_type
    ),
    temporal_processor(
        cfg.denoiser_type,
        cfg.edge_threshold
    ) { }

// Delegating Constructor: Uses edge threshold to build SEALConfig
SEALProcessor::SEALProcessor(int edge_threshold) 
    : SEALProcessor(SEALConfigBuilder().setEdgeThreshold(edge_threshold).build()) {}

// Delegating Constructor: Uses a parameter file path to build SEALConfig
SEALProcessor::SEALProcessor(const std::string param_filepath) 
    : SEALProcessor(SEALConfigBuilder().loadFromYAML(param_filepath).build()) {}

// Simulates the preprocessing done in the Temporal part of SEAL.
void SEALProcessor::temporal_process(cv::Mat& img) {
    if (cfg.seal_denoiser_on) {
        median_filter(img, cfg.denoiser_type);
    }

    if (cfg.seal_edge_filter_on) {
        rl_fast_edge_filter_wide(img, cfg.edge_threshold);
    }
}

// Uses a simplified version of the Good Features to Track (GFTT)
// algorithm to detect keypoints.
// This overload returns the detected corners as a vector of cv::Point2f,
// along with their quality values stored in a separate parallel vector.
// This version provides broader compatibility with older backends that
// do not support cv::KeyPoint.
void SEALProcessor::detect_keypoints(
    cv::InputArray img,
    std::vector<cv::Point2f>& corners,
    std::vector<float>& cornersQuality,
    cv::InputArray mask
) {
    if (!cfg.seal_keypoint_detector_on) {
        corners.clear();
        return;
    }

    detector.detect(img, corners, cornersQuality, mask);
}

// Uses a simplified version of the Good Features to Track (GFTT)
// algorithm to detect keypoints.
// This overload returns the detected keypoints as a vector of cv::KeyPoint.
// This version is a more modern approach and is compatible with backends
// that support cv::KeyPoint.
void SEALProcessor::detect_keypoints(
    cv::InputArray img,
    std::vector<cv::KeyPoint>& keypoints,
    cv::InputArray mask
) {
    if (!cfg.seal_keypoint_detector_on) {
        keypoints.clear();
        return;
    }

    std::vector<cv::Point2f> corners;
    std::vector<float> cornersQuality;
    detector.detect(img, corners, cornersQuality, mask);

    // Convert vector of points for vector of keypoints
    keypoints.resize(corners.size());
    for (size_t i = 0; i < corners.size(); i++)
        keypoints[i] = cv::KeyPoint(corners[i], (float)cfg.gftt_blockSize, -1, cornersQuality[i]);
}

// Uses a simplified version of the Lucan-Kanade Optical Flow
// algorithm to track keypoints from one frame to the other.
void SEALProcessor::track_keypoints(
    cv::InputArray prevImg,
    cv::InputArray nextImg,
    cv::InputArray prevPts,
    cv::InputOutputArray nextPts,
    cv::OutputArray status,
    cv::OutputArray err
) {
    if (!cfg.seal_lk_optical_flow_on) {
        nextPts.release();
        status.release();
        err.release();
        return;
    }

    // Lambda function to extract the first image from a pyramid or use a single Mat directly
    auto extract_first_level = [](cv::InputArray img) -> cv::Mat {
        if (img.kind() == cv::_InputArray::STD_VECTOR_MAT) {
            std::vector<cv::Mat> vec;
            img.getMatVector(vec);
            // Return first level if available
            return (!vec.empty()) ? vec.at(0) : cv::Mat();
        }
        // Directly return the Mat if it's not a pyramid
        return img.getMat();
    };

    cv::Mat prevMat = extract_first_level(prevImg);
    cv::Mat nextMat = extract_first_level(nextImg);

    tracker.track(
        prevMat,
        nextMat,
        prevPts,
        nextPts,
        status,
        err
    );
}

bool SEALProcessor::isDenoiserOn() {
    return cfg.seal_denoiser_on;
}

bool SEALProcessor::isEdgeFilterOn() {
    return cfg.seal_edge_filter_on;
}

bool SEALProcessor::isKeypointDetectorOn() {
    return cfg.seal_keypoint_detector_on;
}

bool SEALProcessor::isKeypointTrackerOn() {
    return cfg.seal_lk_optical_flow_on;
}

int SEALProcessor::buildOpticalFlowPyramid(
    cv::InputArray img,
    cv::OutputArrayOfArrays pyramid
) {
    return tracker.buildOpticalFlowPyramid(
        img,
        pyramid
    );
};
