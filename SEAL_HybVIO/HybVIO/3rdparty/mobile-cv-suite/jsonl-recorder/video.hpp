// private header file
#ifndef JSONL_RECORDER_VIDEO_HPP
#define JSONL_RECORDER_VIDEO_HPP
#include <memory>
#include <string>

namespace cv { class Mat; }

namespace recorder {
struct VideoWriter {
    virtual void write(const cv::Mat &frame) = 0;
    virtual ~VideoWriter();
    static std::unique_ptr<VideoWriter> build(const std::string &prefix, int cameraInd, float fps, const cv::Mat &modelFrame);
};
}
#endif