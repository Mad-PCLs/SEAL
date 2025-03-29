#ifndef RECORDER_H_
#define RECORDER_H_

#include <fstream>
#include <memory>
#include <string>
#include <vector>

// can also use json_fwd here, but then this is less convenient to use as the
// proper include is needed on every file
#include <nlohmann/json.hpp>
#include "types.hpp"

namespace recorder {
class Recorder {
public:
    /**
     * Create new recording. To do multiple recordings during one session, call the constructor multiple times.
     *
     * @param outputPath Platform path to a non-existing file which has write permissions.
     */
    static std::unique_ptr<Recorder> build(const std::string &outputPath);
    /**
     * New recorder that saves both a JSONL recording and AVI video recordings for frame
     * bitmap data (if present). To work, this requires the recorder to be compiled with
     * OpenCV support.
     *
     * @param outputPath JSONL output path
     * @param videoOutputPath The path in which video output is stored for the
     *  first camera. In the stereo/multiple-camera case, each camera will have
     *  its own video file, and their names are derived from this path.
     *  Example
     *      - videoOutputPath: "/path/to/example.avi"
     *      - first camera (cameraInd = 0) file: "/path/to/example.avi"
     *      - 2nd camera (cameraInd = 1) file: "/path/to/example2.avi"
     *      - 3rd camera (cameraInd = 2) file: "/path/to/example3.avi"
     *      - ...
     *  Note that the file name must end in ".avi" due to OpenCV restrictions
     *  (supprting other formats & codecs require nastier dependencies).
     */
    static std::unique_ptr<Recorder> build(const std::string &outputPath, const std::string &videoOutputPath);

    /**
     * @ param output Stream to which output will be written.
     */
    static std::unique_ptr<Recorder> build(std::ostream &output);
    virtual ~Recorder();

    /**
     * Flush and close output file.
     */
    virtual void closeOutputFile() = 0;
    virtual void addGyroscope(const GyroscopeData &d) = 0;
    virtual void addGyroscope(double t, double x, double y, double z) = 0;
    virtual void addAccelerometer(const AccelerometerData &d) = 0;
    virtual void addAccelerometer(double t, double x, double y, double z) = 0;
    virtual void addARKit(const Pose &pose) = 0;
    virtual void addGroundTruth(const Pose &pose) = 0;
    virtual void addOdometryOutput(const Pose &pose, const Vector3d &velocity) = 0;
    virtual void addGps(
        double t,
        double latitude,
        double longitude,
        double horizontalUncertainty,
        double altitude) = 0;

    #ifdef USE_OPENCV_VIDEO_RECORDING
    // Returns reused cv::Mat pointers to be used to store image data for  addFrame(Group) to avoid
    // reallocating memory. Returns false, if not enough free frames are available and
    // adds a dropped frame event to JSONL output file.
    virtual bool getEmptyFrames(size_t number, double time, int width, int height, int type, std::vector<cv::Mat> &out) = 0;
    #endif

    // If addFrame*** fails because of econding/writing can't keep up, call will return false
    // and frame is skipped. A dropped frame entry is added to the JSONL file.
    // When cloneImage is used, cv::Mat is cloned, otherwise existing data is used meaning you shouldn't
    // modify or reuse it.
    virtual bool addFrame(const FrameData &f, bool cloneImage = true) = 0;
    virtual bool addFrameGroup(double t, const std::vector<FrameData> &frames, bool cloneImage = true) = 0;

    /**
     * Write arbitrary serialized JSON into the recording.
     *
     * @param line The serialized data. Needs to be valid JSON.
     */
    virtual void addJsonString(const std::string &line) = 0;

    /**
     * Write arbitrary JSON object into the recording.
     *
     * @param j The JSON object.
     */
    virtual void addJson(const nlohmann::json &j) = 0;

    /**
     * Set reported frames per second for video recording. This does not affect what frame
     * data is actually recorded, only the FPS in the video file, which tells how fast the
     * video should be played.
     * @param fps Frames Per Second, e.g., 24, 25 or 30
     */
    virtual void setVideoRecordingFps(float fps) = 0;
};
} // namespace recorder

#endif // RECORDER_H_
