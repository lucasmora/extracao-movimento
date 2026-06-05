#define MOTION_CORE_EXPORTS
#include "core.h"
#include "processor.h"
#include <cstring>

extern "C" {

MOTION_API int motion_process(const char* input_path, int delay_frames,
                               FrameCallback frame_cb, ProgressCallback progress_cb) {
    Processor proc;
    proc.setInputPath(input_path);
    proc.setDelayFrames(delay_frames);

    if (frame_cb) {
        proc.setFrameCallback([frame_cb](const unsigned char* orig, int ow, int oh,
                                          const unsigned char* prc, int pw, int ph,
                                          int cur, int tot) {
            frame_cb(orig, ow, oh, prc, pw, ph, cur, tot);
        });
    }

    if (progress_cb) {
        proc.setProgressCallback([progress_cb](int pct) {
            progress_cb(pct);
        });
    }

    return proc.process() ? 0 : 1;
}

MOTION_API int motion_get_frame(const char* video_path, int frame_index,
                                 unsigned char** out_data, int* out_width,
                                 int* out_height, int* out_total) {
    cv::VideoCapture cap(video_path);
    if (!cap.isOpened())
        return -1;

    *out_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    *out_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    *out_total = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    if (frame_index >= 0)
        cap.set(cv::CAP_PROP_POS_FRAMES, frame_index);

    cv::Mat frame;
    cap >> frame;
    if (frame.empty())
        return -2;

    size_t dataSize = frame.total() * frame.elemSize();
    *out_data = new unsigned char[dataSize];
    std::memcpy(*out_data, frame.data, dataSize);
    cap.release();
    return 0;
}

MOTION_API void motion_free_buffer(unsigned char* data) {
    delete[] data;
}

} // extern "C"
