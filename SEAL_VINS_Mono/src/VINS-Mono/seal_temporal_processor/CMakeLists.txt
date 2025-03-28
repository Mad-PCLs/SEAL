cmake_minimum_required(VERSION 2.8.3)
project(seal_temporal_processor)

set(CMAKE_BUILD_TYPE "Release")
set(CMAKE_CXX_FLAGS "-std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

# Find dependencies
find_package(catkin REQUIRED COMPONENTS
  roscpp
  sensor_msgs
  cv_bridge
)

# Find OpenCV
find_package(OpenCV 3 REQUIRED)

# Find the SEAL library
find_package(seal REQUIRED)

# Include directories
include_directories(
  ${catkin_INCLUDE_DIRS}
  ${OpenCV_INCLUDE_DIRS}
  ${SEAL_INCLUDE_DIRS}
  include/
)

catkin_package()

# Specify source files for the node
add_executable(seal_temporal_processor
  src/temporal_processor_node.cpp
)

# Link libraries
target_link_libraries(seal_temporal_processor
  ${catkin_LIBRARIES}
  ${OpenCV_LIBS}
  seal::seal
)
