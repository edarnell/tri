#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <cstdio>
#include <iostream>

#define LOW_RES_WIDTH 640
#define LOW_RES_HEIGHT 480
#define HIGH_RES_WIDTH 1920
#define HIGH_RES_HEIGHT 1080

struct buffer {
    void *start;
    size_t length;
};

// Initialize the camera for low-resolution streaming
int initialize_camera(int &fd, const char *device, struct buffer &cam_buffer, size_t &frame_size) {
    fd = open(device, O_RDWR);
    if (fd == -1) {
        perror("Opening video device");
        return -1;
    }

    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = LOW_RES_WIDTH;  // Set to low resolution (640x480)
    fmt.fmt.pix.height = LOW_RES_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  // MJPEG format for streaming
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Setting low-resolution format");
        close(fd);
        return -1;
    }

    // Request buffer
    struct v4l2_requestbuffers req;
    memset(&req, 0, sizeof(req));
    req.count = 1;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd, VIDIOC_REQBUFS, &req) == -1) {
        perror("Requesting Buffer");
        close(fd);
        return -1;
    }

    // Map buffer
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1) {
        perror("Querying Buffer");
        close(fd);
        return -1;
    }

    cam_buffer.length = buf.length;
    cam_buffer.start = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);

    if (cam_buffer.start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return -1;
    }

    frame_size = buf.length;
    return 0;
}

// Capture frame (used for both low and high resolution)
int capture_frame(int fd, struct buffer &cam_buffer, size_t &frame_size) {
    struct v4l2_buffer buf;
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd, VIDIOC_QBUF, &buf) == -1) {
        perror("Queue Buffer");
        return -1;
    }

    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1) {
        perror("Retrieving Frame");
        return -1;
    }

    // Frame is now available in cam_buffer.start
    frame_size = buf.bytesused;  // Actual size of the frame
    return 0;
}

// Capture high-resolution frame for AI training data
int capture_high_res_frame(int fd, struct buffer &cam_buffer, size_t &frame_size) {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = HIGH_RES_WIDTH;  // High resolution
    fmt.fmt.pix.height = HIGH_RES_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  // MJPEG format for high res
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Setting high-resolution format");
        return -1;
    }

    // Capture a single frame
    return capture_frame(fd, cam_buffer, frame_size);  // Reuse the capture_frame logic
}

// Switch back to low resolution after high-res capture
int switch_to_low_res(int fd, struct buffer &cam_buffer, size_t &frame_size) {
    struct v4l2_format fmt;
    memset(&fmt, 0, sizeof(fmt));
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = LOW_RES_WIDTH;
    fmt.fmt.pix.height = LOW_RES_HEIGHT;
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;  // Back to low-res MJPEG format
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

    if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
        perror("Switching back to low-resolution format");
        return -1;
    }

    // Capture the next low-res frame for streaming
    return capture_frame(fd, cam_buffer, frame_size);
}

