#include "camera.h"
#include "cam_server.h"
#include <cstdio>

#define PORT 8080

int main() {
    int fd;
    struct buffer cam_buffer;
    size_t frame_size;

    // Initialize the camera
    if (initialize_camera(fd, "/dev/video0", cam_buffer, frame_size) != 0) {
        printf("Failed to initialize camera\n");
        return 1;
    }

    // Capture a frame to ensure the camera is working
    if (capture_frame(fd, cam_buffer, frame_size) != 0) {
        printf("Failed to capture frame\n");
        return 1;
    }

    // Start the server to stream video
    start_server(PORT, cam_buffer, frame_size);

    // Clean up resources
    munmap(cam_buffer.start, cam_buffer.length);
    close(fd);

    return 0;
}

