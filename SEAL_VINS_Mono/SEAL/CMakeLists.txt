cmake_minimum_required(VERSION 3.5.1)
project(seal VERSION 1.0.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Set default installation prefix to /opt/seal if not specified
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Installation directory" FORCE)
endif()

# Add opencv_internal subdirectory
add_subdirectory(opencv_internal)

# Add cpp files
add_library(seal SHARED
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

# Include directories for the target
target_include_directories(seal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/seal>
    $<INSTALL_INTERFACE:include/seal>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/opencv_internal/include>
)

# Link libraries
target_link_libraries(seal PUBLIC opencv_internal)

# INSTALL
set(PACKAGE_INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include")
set(PACKAGE_LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")

# Install the library and headers
install(TARGETS seal
    EXPORT sealTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(
    DIRECTORY include/seal/
    DESTINATION include/seal
)

# Generate a config file for find_package
include(CMakePackageConfigHelpers)
configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sealConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/sealConfig.cmake
    INSTALL_DESTINATION lib/cmake/seal
)

write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/sealConfigVersion.cmake
    VERSION ${seal_VERSION}
    COMPATIBILITY AnyNewerVersion
)

# Export the library for use in other projects
export(EXPORT sealTargets
    FILE ${CMAKE_CURRENT_BINARY_DIR}/sealTargets.cmake
    NAMESPACE seal::
)

# Install the configuration files
install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/sealConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/sealConfigVersion.cmake
    DESTINATION
        lib/cmake/seal
)

# Install the export file
install(EXPORT sealTargets
    FILE
        sealTargets.cmake
    NAMESPACE
        seal::
    DESTINATION
        lib/cmake/seal
)
