cmake_minimum_required(VERSION 3.5.1)
project(opencv_internal)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic -Werror)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Define the library
add_library(opencv_internal SHARED
    src/corner.cpp
    src/LKTrackerInvoker.cpp
    src/ScharrDerivInvoker.cpp
)

# Expose the include directory for build and install
target_include_directories(opencv_internal PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/opencv_internal>
    $<INSTALL_INTERFACE:include/opencv_internal>
    ${OpenCV_INCLUDE_DIRS}
)

# Link against OpenCV
target_link_libraries(opencv_internal PUBLIC ${OpenCV_LIBS})

# Install the library and headers
install(TARGETS opencv_internal
    EXPORT sealTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include/opencv_internal
)

# Install headers with the "opencv_internal/" prefix
install(
    DIRECTORY include/opencv_internal/
    DESTINATION include/opencv_internal
)
