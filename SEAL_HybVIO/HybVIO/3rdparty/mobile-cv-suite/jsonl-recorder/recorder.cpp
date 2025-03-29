#include <cstdio>
#include "recorder.hpp"
#include "video.hpp"
#include "multithreading/future.hpp"

#ifdef USE_OPENCV_VIDEO_RECORDING
#include "multithreading/framebuffer.hpp"
#include <opencv2/core.hpp>
#endif

#define log_warn std::printf

namespace {
using namespace recorder;
using json = nlohmann::json;

struct RecorderImplementation : public Recorder {
    std::ofstream fileOutput;
    std::ostream &output;
    std::string videoOutputPrefix;
    int frameNumberGroup = 0;
    std::map<int, int> frameNumbers = {};
    std::map<int, std::unique_ptr<VideoWriter> > videoWriters;
    std::map<int, std::unique_ptr<Processor> > videoProcessors;
    float fps = 30;
    std::unique_ptr<Processor> jsonlProcessor;

    #ifdef USE_OPENCV_VIDEO_RECORDING
    std::unique_ptr<recorder::FrameBuffer> frameStore;
    std::vector<cv::Mat> allocatedFrames;
    #endif


    // Preallocate.
    struct Workspace {
        json jGyroscope = R"({
            "time": 0.0,
            "sensor": {
                "type": "gyroscope",
                "values": [0.0, 0.0, 0.0],
                "temperature": 0.0
            }
        })"_json;
        json jAccelerometer = R"({
            "time": 0.0,
            "sensor": {
                "type": "accelerometer",
                "values": [0.0, 0.0, 0.0],
                "temperature": 0.0
            }
        })"_json;
        json jGps = R"({
            "time": 0.0,
            "gps": {
                "latitude": 0.0,
                "longitude": 0.0,
                "accuracy": 0.0,
                "altitude": 0.0
            }
        })"_json;
        json jOutput = R"({
            "time": 0.0,
            "output": {
                "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
                "orientation": { "w": 0.0, "x": 0.0, "y": 0.0, "z": 0.0 },
                "velocity": { "x": 0.0, "y": 0.0, "z": 0.0 }
            }
        })"_json;
        json jGroundTruth = R"({
            "time": 0.0,
            "groundTruth": {
                "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
                "orientation": { "w": 0.0, "x": 0.0, "y": 0.0, "z": 0.0 }
            }
        })"_json;
        json jARKit = R"({
            "time": 0.0,
            "ARKit": {
                "position": { "x": 0.0, "y": 0.0, "z": 0.0 },
                "orientation": { "w": 0.0, "x": 0.0, "y": 0.0, "z": 0.0 }
            }
        })"_json;
        json jFrame = R"({
            "time": 0.0,
            "cameraInd": 0,
            "number": 0,
            "cameraParameters": {
                "focalLength": 0.0,
                "principalPointX": 0.0,
                "principalPointY": 0.0
            }
        })"_json;
        json jFrameGroup = R"({
            "time": 0.0,
            "number": 0,
            "frames": []
        })"_json;
        json jFrameDrop = R"({
            "time": 0.0,
            "droppedFrame": true
        })"_json;
    } workspace;

    RecorderImplementation(std::ostream &output) :
        fileOutput(),
        output(output)
    {
        init();
    }

    RecorderImplementation(const std::string &outputPath) :
            fileOutput(outputPath),
            output(this->fileOutput)
    {
        init();
    }

    RecorderImplementation(const std::string &outputPath, const std::string &videoOutputPrefix) :
            fileOutput(outputPath),
            output(this->fileOutput),
            videoOutputPrefix(videoOutputPrefix)
    {
        init();
    }

    void init() {
        output.precision(10);
        jsonlProcessor = Processor::createThreadPool(1);
        #ifdef USE_OPENCV_VIDEO_RECORDING
        constexpr std::size_t CAPACITY_INCREASE = 4;
        // Shared between stereo, i.e. MAX_CAPACITY mono frames, or MAX_CAPACITY/2 stereo pairs can
        // be buffered in memory before frame skipping occurs if video encoding cannot keep up
        constexpr std::size_t MAX_CAPACITY = 20;
        frameStore = std::make_unique<recorder::FrameBuffer>(
            CAPACITY_INCREASE, MAX_CAPACITY
        );
        #endif
    }

    void closeOutputFile() final {
        fileOutput.close();
    }

    void addGyroscope(const GyroscopeData &d) final {
        jsonlProcessor->enqueue([this, d]() {
            workspace.jGyroscope["time"] = d.t;
            workspace.jGyroscope["sensor"]["values"] = { d.x, d.y, d.z };
            workspace.jGyroscope["sensor"].erase("temperature");
            if (d.temperature > 0.0) {
            workspace.jGyroscope["sensor"]["temperature"] = d.temperature;
            }
            output << workspace.jGyroscope.dump() << std::endl;
        });
    }

    void addGyroscope(double t, double x, double y, double z) final {
        GyroscopeData d {
          /* .t = */ t,
          /* .x = */ x,
          /* .y = */ y,
          /* .z = */ z,
          /* .temperature = */ -1.0,
        };
        addGyroscope(d);
    }

    void addAccelerometer(const AccelerometerData &d) final {
        jsonlProcessor->enqueue([this, d]() {
            workspace.jAccelerometer["time"] = d.t;
            workspace.jAccelerometer["sensor"]["values"] = { d.x, d.y, d.z };
            workspace.jAccelerometer["sensor"].erase("temperature");
            if (d.temperature > 0.0) {
            workspace.jAccelerometer["sensor"]["temperature"] = d.temperature;
            }
            output << workspace.jAccelerometer.dump() << std::endl;
        });
    }

    void addAccelerometer(double t, double x, double y, double z) final {
        AccelerometerData d {
          /* .t = */ t,
          /* .x = */ x,
          /* .y = */ y,
          /* .z = */ z,
          /* .temperature = */ -1.0
        };
        addAccelerometer(d);
    }

    void setFrame(const FrameData &f) {
        workspace.jFrame["time"] = f.t;
        workspace.jFrame["cameraInd"] = f.cameraInd;

        workspace.jFrame.erase("cameraParameters");
        if (f.focalLengthX > 0.0) {
            workspace.jFrame["cameraParameters"]["focalLengthX"] = f.focalLengthX;
        }
        if (f.focalLengthY > 0.0) {
            workspace.jFrame["cameraParameters"]["focalLengthY"] = f.focalLengthY;
        }
        if (f.px > 0.0 && f.py > 0.0) {
            workspace.jFrame["cameraParameters"]["principalPointX"] = f.px;
            workspace.jFrame["cameraParameters"]["principalPointY"] = f.py;
        }
    }

    void frameDrop(double time) {
        jsonlProcessor->enqueue([this, time]() {
            workspace.jFrameDrop["time"] = time;
            output << workspace.jFrameDrop.dump() << std::endl;
        });
    }

    #ifdef USE_OPENCV_VIDEO_RECORDING
    bool getEmptyFrames(size_t number, double time, int width, int height, int type, std::vector<cv::Mat> &out) {
        out.resize(number);
        for (size_t i = 0; i < number; i++) {
            auto frame = frameStore->next(height, width, type);
            if (!frame) {
                frameDrop(time);
                out.clear(); // Free already allocated frames
                return false;
            }
            out[i] = *frame;
        }
        return true;
    }

    bool allocateAndWriteVideo(const std::vector<FrameData> &frames, bool cloneImage) {
        allocatedFrames.clear();
        // Allocate all frames, so if we don't have space for second frame of stereo, drop both
        for (auto f : frames) {
            if (f.frameData == nullptr) continue;
            auto frameUniqPtr = frameStore->next(f.frameData->rows, f.frameData->cols, f.frameData->type());
            if (!frameUniqPtr) return false;
            cv::Mat allocatedFrameData = *frameUniqPtr.get();
            if (cloneImage)
                f.frameData->copyTo(allocatedFrameData);
            else
                allocatedFrameData = *f.frameData; // Doesn't copy data, cv::Mat as smart pointer
            allocatedFrames.push_back(allocatedFrameData);
        }
        for (size_t i = 0; i < frames.size(); i++) {
            if (frames[i].frameData == nullptr) continue;
            cv::Mat allocatedFrameData = allocatedFrames[i];
            int cameraInd = frames[i].cameraInd;
            if (!videoWriters.count(cameraInd)) {
                videoWriters[cameraInd] = VideoWriter::build(videoOutputPrefix, cameraInd, fps, allocatedFrameData);
                videoProcessors[cameraInd] = Processor::createThreadPool(1);
            }
            videoProcessors.at(cameraInd)->enqueue([this, cameraInd, allocatedFrameData]() {
                videoWriters.at(cameraInd)->write(allocatedFrameData);
            });
        }
        return true;
    }
    #endif

    bool addFrame(const FrameData &f, bool cloneImage) final {
        #ifdef USE_OPENCV_VIDEO_RECORDING
        if (!videoOutputPrefix.empty()) {
            const std::vector<FrameData> &frames{f};
            if (!allocateAndWriteVideo(frames, cloneImage)) {
                frameDrop(f.t);
                return false;
            }
        }
        #endif

        jsonlProcessor->enqueue([this, f]() { // f.frameData pointer no longer valid
            setFrame(f);
            workspace.jFrame["number"] = frameNumberGroup;
            workspace.jFrameGroup["time"] = f.t;
            workspace.jFrameGroup["number"] = frameNumberGroup;
            workspace.jFrameGroup["frames"] = {};
            workspace.jFrameGroup["frames"].push_back(workspace.jFrame);
            output << workspace.jFrameGroup.dump() << std::endl;
            frameNumberGroup++;
        });
        return true;
    }

    bool addFrameGroup(double t, const std::vector<FrameData> &frames, bool cloneImage) final {
        #ifdef USE_OPENCV_VIDEO_RECORDING
        if (!videoOutputPrefix.empty()) {
            if (!allocateAndWriteVideo(frames, cloneImage)) {
                frameDrop(t);
                return false;
            }
        }
        #endif

        jsonlProcessor->enqueue([this, t, frames]() {
            workspace.jFrameGroup["time"] = t;
            workspace.jFrameGroup["number"] = frameNumberGroup;
            workspace.jFrameGroup["frames"] = {};
            for (auto f : frames) { // f.frameData pointer no longer valid
                // Track frame numbers of each camera because some frame groups
                // may only contain output from some of the cameras (happens on iOS).
                try {
                    frameNumbers.at(f.cameraInd)++;
                } catch (const std::out_of_range &e) {
                    frameNumbers[f.cameraInd] = 0;
                }
                setFrame(f);
                workspace.jFrame["number"] = frameNumbers[f.cameraInd];
                workspace.jFrameGroup["frames"].push_back(workspace.jFrame);
            }
            output << workspace.jFrameGroup.dump() << std::endl;
            frameNumberGroup++;
        });
        return true;
    }

    void setPose(const Pose &pose, json &j, const std::string &name, bool hasOrientation) {
        j["time"] = pose.time;
        j[name]["position"]["x"] = pose.position.x;
        j[name]["position"]["y"] = pose.position.y;
        j[name]["position"]["z"] = pose.position.z;

        j[name].erase("orientation");
        if (hasOrientation) {
            j[name]["orientation"]["w"] = pose.orientation.w;
            j[name]["orientation"]["x"] = pose.orientation.x;
            j[name]["orientation"]["y"] = pose.orientation.y;
            j[name]["orientation"]["z"] = pose.orientation.z;
        }
    }

    void addARKit(const Pose &pose) final {
        jsonlProcessor->enqueue([this, pose]() {
            setPose(pose, workspace.jARKit, "ARKit", false);
            output << workspace.jARKit.dump() << std::endl;
        });
    }

    void addGroundTruth(const Pose &pose) final {
        jsonlProcessor->enqueue([this, pose]() {
            setPose(pose, workspace.jGroundTruth, "groundTruth", false);
            output << workspace.jGroundTruth.dump() << std::endl;
        });
    }

    void addOdometryOutput(const Pose &pose, const Vector3d &velocity) final {
        jsonlProcessor->enqueue([this, pose, velocity]() {
            setPose(pose, workspace.jOutput, "output", true);
            workspace.jOutput["output"]["velocity"]["x"] = velocity.x;
            workspace.jOutput["output"]["velocity"]["y"] = velocity.y;
            workspace.jOutput["output"]["velocity"]["z"] = velocity.z;
            output << workspace.jOutput.dump() << std::endl;
        });
    }

    void addGps(
        double t,
        double latitude,
        double longitude,
        double horizontalUncertainty,
        double altitude) final
    {
        jsonlProcessor->enqueue([this, t, latitude, longitude, horizontalUncertainty, altitude]() {
            workspace.jGps["time"] = t;
            workspace.jGps["gps"]["latitude"] = latitude;
            workspace.jGps["gps"]["longitude"] = longitude;
            // We have no standard for what "accuracy" means.
            workspace.jGps["gps"]["accuracy"] = horizontalUncertainty;
            workspace.jGps["gps"]["altitude"] = altitude;
            output << workspace.jGps.dump() << std::endl;
        });
    }

    void addJsonString(const std::string &line) final {
        jsonlProcessor->enqueue([this, line]() {
            json j;
            try {
                j = json::parse(line);
            } catch (const nlohmann::detail::parse_error &e) {
                log_warn("recorder addLine(): Skipping invalid JSON: %s", line.c_str());
                return;
            }

            // Make sure output is exactly one line.
            size_t n = line.find('\n');
            if (n == std::string::npos) {
                output << line << std::endl;
            } else if (n + 1 < line.size()) {
                // Re-serialize multiline input.
                output << j.dump() << std::endl;
            } else {
                output << line;
            }
        });
    }

    void addJson(const json &j) final {
        jsonlProcessor->enqueue([this, j]() {
            output << j.dump() << std::endl;
        });
    }

    void setVideoRecordingFps(float f) final {
        fps = f;
    }
};

} // anonymous namespace

namespace recorder {

std::unique_ptr<Recorder> Recorder::build(const std::string &outputPath) {
    return std::unique_ptr<Recorder>(new RecorderImplementation(outputPath));
}

std::unique_ptr<Recorder> Recorder::build(const std::string &outputPath, const std::string &videoOutputPath) {
    std::string videoOutputPrefix = "";
    if (!videoOutputPath.empty()) {
        assert(videoOutputPath.size() >= 4);
        assert(videoOutputPath.substr(videoOutputPath.size() - 4) == ".avi");
        videoOutputPrefix = videoOutputPath.substr(0, videoOutputPath.size() - 4);
    }
    return std::unique_ptr<Recorder>(new RecorderImplementation(outputPath, videoOutputPrefix));
}

std::unique_ptr<Recorder> Recorder::build(std::ostream &output) {
    return std::unique_ptr<Recorder>(new RecorderImplementation(output));
}

Recorder::~Recorder() = default;

} // namespace recorder
