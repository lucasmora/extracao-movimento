#pragma once

#ifdef MOTION_CORE_EXPORTS
#  define MOTION_API __declspec(dllexport)
#else
#  define MOTION_API __declspec(dllimport)
#endif

extern "C" {

typedef void (*FrameCallback)(const unsigned char* original, int orig_w, int orig_h,
                               const unsigned char* processed, int proc_w, int proc_h,
                               int current_frame, int total_frames);

typedef void (*ProgressCallback)(int percent);

MOTION_API int motion_process(const char* input_path, int delay_frames,
                               FrameCallback frame_cb, ProgressCallback progress_cb);

MOTION_API int motion_get_frame(const char* video_path, int frame_index,
                                 unsigned char** out_data, int* out_width,
                                 int* out_height, int* out_total);

MOTION_API void motion_free_buffer(unsigned char* data);

} // extern "C"
