cmake_minimum_required(VERSION 3.10)
project(opencv_internal)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Define the library
add_library(opencv_internal STATIC
    src/corner.cpp
    src/LKTrackerInvoker.cpp
    src/ScharrDerivInvoker.cpp
)

# Make sure headers are installed with the "opencv_internal/" prefix
install(
    DIRECTORY include/
    DESTINATION include/opencv_internal
    COMPONENT my_dependencies
)

# Expose the include directory for build and install
target_include_directories(opencv_internal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/opencv_internal>
    ${OpenCV_INCLUDE_DIRS}
)

# Link against OpenCV
target_link_libraries(opencv_internal PUBLIC ${OpenCV_LIBS})
