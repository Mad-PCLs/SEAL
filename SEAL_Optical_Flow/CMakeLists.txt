cmake_minimum_required(VERSION 3.10)
project(SEAL_Optical_Flow)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${OpenCV_INCLUDE_DIRS})
# include_directories(SEAL)

##############################
# PCL addition: Include SEAL #
##############################
add_subdirectory(SEAL)
# Ensure SEAL headers are available to other parts of HybVIO
include_directories(${CMAKE_SOURCE_DIR}/SEAL/include)

link_libraries(${OpenCV_LIBS} seal)

add_executable(SEAL_Optical_Flow
    src/main.cpp
)
