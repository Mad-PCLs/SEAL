# jsonl-recorder

Can be used to record sensor data and other data such as VIO frame metadata or GPS locations to a JSONL file.
Usage
 * `#include <jsonl-recorder/recorder.hpp>`
 * link with `-ljsonl-recorder`.

Dependencies (included as submodules)
 * https://github.com/nlohmann/json
 * optional for tests: https://github.com/catchorg/Catch2.git

Recording video data to `.avi` files, in addition to the JSONL logs is also supported.
This requires compiling with `-DUSE_OPENCV_VIDEO_RECORDING=ON` and OpenCV has to be available.

## Installation

### CMake project

For example
```cmake
# path to a submodule pointing to this repository
add_subdirectory("jsonl-recorder")
target_link_libraries(YOUR_TARGET jsonl-recorder)
```

### Standalone

```sh
mkdir target; cd target
cmake -DCMAKE_INSTALL_PREFIX=/where/i/want/it/installed ..
cmake --build . --config Release --target install
```
`CMAKE_INSTALL_PREFIX` is optional but recommended, unless you like
to `sudo make install` everything and enjoy debugging random issues caused
by this approach later.

### Tests

After `cmake ...`, run `cmake --build . --target jsonl-recorder-tests` and `ctest`
(or just `make && ctest`)
