// This creates the main() function for the binary that runs the tests.
#define CATCH_CONFIG_MAIN

#include <catch2/catch.hpp>
#include <sstream>

#include "recorder.hpp"

TEST_CASE( "recorder", "[jsonl-recorder]" ) {
    // Write to file:
    // auto r = recorder::Recorder::build("test.json");
    // Write to stdout:
    // auto r = recorder::Recorder::build(std::cout);

    std::ostringstream output;
    auto r = recorder::Recorder::build(output);

    REQUIRE( output.str().find("gyroscope") == std::string::npos );

    r->addGyroscope(0.1, 0.2, 0.3, 0.4);
    r->addGyroscope(0.2, 0.3, 0.5, 0.5);

    REQUIRE( output.str().find("gyroscope") != std::string::npos );

    auto f0 = recorder::FrameData {
        .t = 0.0,
        .cameraInd = 0,
        .focalLengthX = 1000.0,
        .focalLengthY = 1000.0,
        .px = 640.0,
        .py = 360.0
    };
    auto f1 = recorder::FrameData {
        .t = 0.0,
        .cameraInd = 1,
        .focalLengthX = 1001.0,
        .focalLengthY = 1001.0,
        .px = 640.0,
        .py = 360.0
    };

    r->addFrame(f0);
    r->addFrame(f1);

    r->addFrameGroup(0.15, { f0, f1 });
    r->addFrameGroup(0.20, { f1 });
    r->addFrameGroup(0.25, { f1, f0 });

    r->addJson({{ "time", 0.11 }, { "data", {{ "one", 1 }, { "two", 2 }}}});
    r->addJsonString(R"({
        "time": 0.11,
        "data": {
            "one": 1,
            "two": 2
        }
    })");

    // std::cout << output.str() << std::endl;
}
