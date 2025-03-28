#include "SEALConfigBuilder.hpp"

SEALConfigBuilder& SEALConfigBuilder::setHighLevelGuards(
    bool seal_on,
    bool seal_denoiser_on,
    bool seal_edge_filter_on,
    bool seal_keypoint_detector_on,
    bool seal_lk_optical_flow_on
) {
    config.seal_on = seal_on;
    if (!seal_on) {
        config.seal_denoiser_on = false;
        config.seal_edge_filter_on = false;
        config.seal_keypoint_detector_on = false;
        config.seal_lk_optical_flow_on = false;
    }
    else {
        config.seal_denoiser_on = seal_denoiser_on;
        config.seal_edge_filter_on = seal_edge_filter_on;
        config.seal_keypoint_detector_on = seal_keypoint_detector_on;
        config.seal_lk_optical_flow_on = seal_lk_optical_flow_on;
    }
    return *this;
}

SEALConfigBuilder& SEALConfigBuilder::setMedianFilterParams(
    DenoiserType denoiser_type
) {
    config.denoiser_type = denoiser_type;
    return *this;
}

SEALConfigBuilder& SEALConfigBuilder::setEdgeThreshold(int threshold) {
    config.edge_threshold = threshold;
    return *this;
}

SEALConfigBuilder& SEALConfigBuilder::setGFTTParams(
    GFTTCornerDerivativeType gftt_corner_derivative_type,
    int maxCorners,
    double qualityLevel,
    double minDistance,
    int blockSize,
    int gradientSize,
    bool useHarrisDetector,
    double k
) {
    config.gftt_corner_derivative_type = gftt_corner_derivative_type;
    config.gftt_maxCorners = maxCorners;
    config.gftt_qualityLevel = qualityLevel;
    config.gftt_minDistance = minDistance;
    config.gftt_blockSize = blockSize;
    config.gftt_gradientSize = gradientSize;
    config.gftt_useHarrisDetector = useHarrisDetector;
    config.gftt_k = k;
    return *this;
}

SEALConfigBuilder& SEALConfigBuilder::setOpticalFlowParams(
    LKSpatialDerivativeType lk_spatialDerivativeType,
    LKPyrDownFilterType lk_pyrDownFilterType,
    cv::Size winSize,
    int maxLevel,
    cv::TermCriteria criteria,
    int lk_flags,
    float lk_minEigThreshold,
    bool lk_useInitialFlow
) {
    config.lk_spatialDerivativeType = lk_spatialDerivativeType;
    config.lk_pyrDownFilterType = lk_pyrDownFilterType;
    config.lk_winSize = winSize;
    config.lk_maxLevel = maxLevel;
    config.lk_termCriteria = criteria;
    config.lk_flags = lk_flags;
    config.lk_minEigThreshold = lk_minEigThreshold;
    config.lk_useInitialFlow = lk_useInitialFlow;
    return *this;
}

SEALConfigBuilder& SEALConfigBuilder::loadFromYAML(
    const std::string& filename
) {
    cv::FileStorage fs(filename, cv::FileStorage::READ);
    if (!fs.isOpened()) {
        std::cerr << "Error: Could not open YAML file " << filename << std::endl;
        throw std::ios_base::failure("Failed to open the file: " + filename);
    }

    // Helper lambda function to check if a key exists
    auto checkKey = [&](const std::string& key) {
        if (!fs[key].isNone()) return true;
        std::cerr << "Error: Missing required key '" << key << "' in YAML file " << filename << std::endl;
        throw std::runtime_error("Missing required key: " + key);
    };

    // High level guards
    checkKey("seal_on");
    fs["seal_on"] >> config.seal_on;
    std::string tmp;
    if (!config.seal_on) {
        config.seal_denoiser_on = false;
        config.seal_edge_filter_on = false;
        config.seal_keypoint_detector_on = false;
        config.seal_lk_optical_flow_on = false;
    }
    else {
        checkKey("seal_denoiser_on");
        checkKey("seal_edge_filter_on");
        checkKey("seal_keypoint_detector_on");
        checkKey("seal_lk_optical_flow_on");
        fs["seal_denoiser_on"] >> config.seal_denoiser_on;
        fs["seal_edge_filter_on"] >> config.seal_edge_filter_on;
        fs["seal_keypoint_detector_on"] >> config.seal_keypoint_detector_on;
        fs["seal_lk_optical_flow_on"] >> config.seal_lk_optical_flow_on;
    }

    // Denoiser params
    checkKey("denoiser_type");
    std::string denoiser_type;
    fs["denoiser_type"] >> denoiser_type;
    config.denoiser_type = getDenoiserTypeFromString(denoiser_type);

    // Edge params
    checkKey("edge_threshold");
    fs["edge_threshold"] >> config.edge_threshold;

    // Keypoint Detection params
    checkKey("gftt_corner_derivative_type");
    std::string gftt_type;
    fs["gftt_corner_derivative_type"] >> gftt_type;
    config.gftt_corner_derivative_type = getGFTTCornerDerivativeTypeFromString(
        gftt_type
    );

    checkKey("gftt_max_corners");
    checkKey("gftt_quality_level");
    checkKey("gftt_min_distance");
    checkKey("gftt_block_size");
    checkKey("gftt_gradient_size");
    checkKey("gftt_use_harris_detector");
    checkKey("gftt_k");
    fs["gftt_max_corners"] >> config.gftt_maxCorners;
    fs["gftt_quality_level"] >> config.gftt_qualityLevel;
    fs["gftt_min_distance"] >> config.gftt_minDistance;
    fs["gftt_block_size"] >> config.gftt_blockSize;
    fs["gftt_gradient_size"] >> config.gftt_gradientSize;
    fs["gftt_use_harris_detector"] >> config.gftt_useHarrisDetector;
    fs["gftt_k"] >> config.gftt_k;

    // Keypoint Tracking params
    checkKey("lk_spatial_derivative_type");
    std::string lk_spatial_derivative_type;
    fs["lk_spatial_derivative_type"] >> lk_spatial_derivative_type;
    config.lk_spatialDerivativeType = getLKSpatialDerivativeTypeFromString(
        lk_spatial_derivative_type
    );

    checkKey("lk_pyr_down_filter_type");
    std::string lk_pyr_down_filter_type;
    fs["lk_pyr_down_filter_type"] >> lk_pyr_down_filter_type;
    config.lk_pyrDownFilterType = getLKPyrDownFilterTypeFromString(
        lk_pyr_down_filter_type
    );

    checkKey("lk_win_size_width");
    checkKey("lk_win_size_height");
    int width, height;
    fs["lk_win_size_width"] >> width;
    fs["lk_win_size_height"] >> height;
    config.lk_winSize = cv::Size(width, height);

    checkKey("lk_max_level");
    fs["lk_max_level"] >> config.lk_maxLevel;

    checkKey("lk_term_criteria_max_count");
    checkKey("lk_term_criteria_eps");
    int termMaxCount;
    double termEps;
    fs["lk_term_criteria_max_count"] >> termMaxCount;
    fs["lk_term_criteria_eps"] >> termEps;
    config.lk_termCriteria = cv::TermCriteria(
        cv::TermCriteria::COUNT + cv::TermCriteria::EPS,
        termMaxCount,
        termEps
    );

    checkKey("lk_flags");
    checkKey("lk_min_eig_threshold");
    checkKey("lk_use_initial_flow");
    fs["lk_flags"] >> config.lk_flags;
    fs["lk_min_eig_threshold"] >> config.lk_minEigThreshold;
    fs["lk_use_initial_flow"] >> config.lk_useInitialFlow;

    fs.release();
    return *this;
}

SEALConfig SEALConfigBuilder::build() {
    return config;
}
