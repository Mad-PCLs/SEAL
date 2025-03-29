#include "recorder.hpp"
#include "video.hpp"

#ifdef USE_OPENCV_VIDEO_RECORDING
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <sstream>

namespace recorder {
namespace {
std::unique_ptr<cv::VideoWriter> buildOpenCVVideoWriter(const std::string &path, float fps, const cv::Mat &modelFrame) {
    assert(fps > 0.0);
    assert(!path.empty());
    const auto codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    // This is the only thing we can write without FFMPEG on Android
    // The path name should end with .avi
    const auto backend = cv::CAP_OPENCV_MJPEG;
    const bool isColor = modelFrame.channels() > 1;
    {
        // OpenCV writer gives no errors even if it is unable to open file and
        // write frames to it, so test writing by ourselves.
        std::ofstream video(path);
        assert(video.is_open() && "unable to open video file for writing");
    }
    // log_info("recording %s video stream to %s", isColor ? "color" : "gray", path.c_str());
    auto writer = std::make_unique<cv::VideoWriter>(
            path, backend, codec, fps, modelFrame.size(), isColor);
    assert(writer && "failed to create video writer");
    // TODO: set video quality
    return writer;
}

std::string videoOutputPath(const std::string &prefix, int cameraInd) {
    std::ostringstream oss;
    oss << prefix;
    if (cameraInd != 0) {
        oss << (cameraInd + 1);
    }
    oss << ".avi"; // must be .avi so OpenCV can record this without FFMPEG
    return oss.str();
}

struct VideoWriterImplementation : public VideoWriter {
    const std::unique_ptr<cv::VideoWriter> writer;
    cv::Mat outputFrame;

    VideoWriterImplementation(std::unique_ptr<cv::VideoWriter> writer) : writer(std::move(writer)) {}

    void write(const cv::Mat &frame) final {
        if (frame.channels() == 4) {
            cv::cvtColor(frame, outputFrame, cv::COLOR_BGRA2BGR);
            // This took a while to debug: if the image has 3 channels, the
            // default channel order assumed by OpenCV image IO functions
            // is BGR (which everybody on the internet warns you about).
            // However, if there are 4 channels, at least this particular
            // function (on Android) assumes the color order RGBA
            writer->write(outputFrame);
        } else {
            writer->write(frame);
        }
    }
};
}

std::unique_ptr<VideoWriter> VideoWriter::build(const std::string &prefix, int cameraInd, float fps, const cv::Mat &modelFrame) {
    auto fn = videoOutputPath(prefix, cameraInd);
    return std::unique_ptr<VideoWriter>(new VideoWriterImplementation(buildOpenCVVideoWriter(fn, fps, modelFrame)));
}
}

#else
namespace recorder {
std::unique_ptr<VideoWriter> VideoWriter::build(const std::string &prefix, int cameraInd, float fps, const cv::Mat &modelFrame) {
    (void)prefix;
    (void)cameraInd;
    (void)fps;
    (void)modelFrame;
    assert(false && "not built with OpenCV video recording support");
    return nullptr;
}
}
#endif

recorder::VideoWriter::~VideoWriter() = default;
