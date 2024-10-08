#ifndef CAMERA_H
#define CAMERA_H

#include <cstddef>  // For size_t

struct buffer {
    void *start;
    size_t length;
};

// Function declarations for camera operations
int initialize_camera(int &fd, const char *device, struct buffer &cam_buffer, size_t &frame_size);
int capture_frame(int fd, struct buffer &cam_buffer, size_t &frame_size);
int capture_high_res_frame(int fd, struct buffer &cam_buffer, size_t &frame_size);
int switch_to_low_res(int fd, struct buffer &cam_buffer, size_t &frame_size);

#endif  // CAMERA_H

