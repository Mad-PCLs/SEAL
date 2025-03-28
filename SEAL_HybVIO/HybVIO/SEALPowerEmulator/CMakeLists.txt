cmake_minimum_required(VERSION 3.10)
project(SEALPowerEmulator)

# Enable C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find the filesystem library (needed for std::filesystem)
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Inherit HybVIO's include directories (Cereal is already there)
include_directories(${CMAKE_SOURCE_DIR}/3rdparty/mobile-cv-suite/cereal/include)

# Define the SEALPowerEmulator library
add_library(SEALPowerEmulator STATIC
    SEALPowerEmulator.cpp
)

# Link dependencies (Cereal is header-only, but filesystem needs linking)
target_link_libraries(SEALPowerEmulator PRIVATE Threads::Threads)

# Make this library available to other projects
target_include_directories(SEALPowerEmulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
