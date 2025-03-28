cmake_minimum_required(VERSION 3.10)
project(seal)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# OpenCV flags
add_definitions(-DOPENCV_LOG_LEVEL=CV_LOG_LEVEL_ERROR)

# Add opencv_interal subdirectory
add_subdirectory(opencv_internal)

# Add cpp files
add_library(seal
    src/types/DenoiserTypes.cpp
    src/types/GFTTCornerDerivativeTypes.cpp
    src/types/LKSpatialDerivativeTypes.cpp
    src/types/LKPyrDownFilterTypes.cpp
    src/temporal_processing/denoise.cpp
    src/temporal_processing/edge_filter.cpp
    src/temporal_processing/TemporalProcessor.cpp
    src/keypoint_detection/corner.cpp
    src/keypoint_detection/gftt.cpp
    src/keypoint_detection/gradients.cpp
    src/keypoint_detection/KeypointDetector.cpp
    src/keypoint_tracking/gradients.cpp
    src/keypoint_tracking/pyramids.cpp
    src/keypoint_tracking/SparsePyrLKOpticalFlowSealImpl.cpp
    src/keypoint_tracking/KeypointTracker.cpp
    src/SEALConfigBuilder.cpp
    src/SEALProcessor.cpp
)

# Include directories
target_include_directories(seal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(seal PUBLIC opencv_internal)
