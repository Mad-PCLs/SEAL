cmake_minimum_required(VERSION 3.0)
set(LIBNAME jsonl-recorder)
project(${LIBNAME})

set(CMAKE_CXX_STANDARD 14)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -O2")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -O2")
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

option(USE_OPENCV_VIDEO_RECORDING "Video recording with OpenCV" OFF)
add_library(${LIBNAME}
  multithreading/future.cpp
  multithreading/queue.cpp
  recorder.cpp
  video.cpp
  jsonl_reader.cpp)
set_target_properties(${LIBNAME} PROPERTIES PUBLIC_HEADER "recorder.hpp;types.hpp;jsonl_reader.hpp")
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json ${CMAKE_CURRENT_BINARY_DIR}/nlohmann_json)
find_package(Threads)
set(JSONL_RECORDER_LIBRARY_DEPS nlohmann_json::nlohmann_json Threads::Threads)
if (USE_OPENCV_VIDEO_RECORDING)
    find_package(OpenCV REQUIRED)
    target_compile_definitions(${LIBNAME} PRIVATE "-DUSE_OPENCV_VIDEO_RECORDING")
    list(APPEND JSONL_RECORDER_LIBRARY_DEPS ${OpenCV_LIBS})
    target_include_directories(${LIBNAME} PRIVATE ${OpenCV_INCLUDE_DIRS})
endif()
target_link_libraries(${LIBNAME} PUBLIC ${JSONL_RECORDER_LIBRARY_DEPS})

install(TARGETS ${LIBNAME}
  ARCHIVE DESTINATION lib
  PUBLIC_HEADER DESTINATION include/${LIBNAME})

enable_testing()
if (BUILD_TESTING)
  set(TEST_NAME ${LIBNAME}-tests)
  add_executable(${TEST_NAME} test.cpp)
  target_link_libraries(${TEST_NAME} ${LIBNAME})
  target_include_directories(${TEST_NAME} PRIVATE Catch2/single_include)
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endif()
