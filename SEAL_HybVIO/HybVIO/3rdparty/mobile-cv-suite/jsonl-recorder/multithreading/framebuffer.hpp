#ifndef RECORDER_FRAME_BUFFER
#define RECORDER_FRAME_BUFFER

#include <iostream>
#include <opencv2/core.hpp>

namespace recorder {
/**
 * A buffer and custom allocation mechanism for OpenCV frames. Features
 *  - lazy initialization to automatically determine the capacity
 *  - if the cacacity is exceeded, allocates more memory and re-uses that
 *  - automatic reference counting (using OpenCV Mat internal refcounter) to
 *    check which slots are free
 */
class FrameBuffer {
private:
    static constexpr std::size_t DEFAULT_CAPACITY_INCREASE = 4;
    const std::size_t capacityIncrease;
    const std::size_t maxCapacity;

    std::vector<cv::Mat> buf;
    std::size_t counter = 0;

    bool increaseCapacity(int rows, int cols, int type) {
        auto n0 = buf.size();
        if (n0 == maxCapacity)
            return false;
        n0 = std::min(n0 + capacityIncrease, maxCapacity);
        while (buf.size() < n0) {
            buf.emplace_back(cv::Mat(rows, cols, type));
        }
        return true;
    }

public:
    FrameBuffer(
        std::size_t capacityIncrease = DEFAULT_CAPACITY_INCREASE,
        std::size_t maxCapacity = DEFAULT_CAPACITY_INCREASE * 5)
    :
        capacityIncrease(capacityIncrease),
        maxCapacity(maxCapacity) {}

    /**
     * Return a uniq pointer to an free slot. If FrameBuffer is full, pointer is empty.
    */
    std::unique_ptr<cv::Mat> next(int rows, int cols, int type) {
        if (buf.empty()) increaseCapacity(rows, cols, type);
        for (std::size_t failures = 0; failures < buf.size(); failures++) {
            auto &m = buf[(counter = (counter + 1) % buf.size())];
            assert(m.u && "u missing");
            // NOTE: this is not part of the OpenCV public API so use with care
            // This version should be thread-safe
            const auto refcount = CV_XADD(&m.u->refcount, 0);
            assert(refcount >= 1);
            if (refcount == 1) {
                return std::make_unique<cv::Mat>(m);
            }
        }
        if (increaseCapacity(rows, cols, type)) {
            return next(rows, cols, type);
        } else {
            return std::unique_ptr<cv::Mat>(nullptr);
        }
    }
};

} // namespace recorder

#endif
