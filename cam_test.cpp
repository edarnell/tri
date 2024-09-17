#include "camera.h"
#include "cam_server.h"  // Assuming cam_server is for streaming
#include <sys/mman.h>  // For munmap
#include <cstdio>
#include <sys/mman.h>
#include <unistd.h>  // For sleep()

#define PORT 8080

int main() {
    int fd;
    struct buffer cam_buffer;
    size_t frame_size;

    // Initialize the camera for low-resolution streaming
    if (initialize_camera(fd, "/dev/video0", cam_buffer, frame_size) != 0) {
        printf("Failed to initialize camera\n");
        return 1;
    }

    // Capture and stream at low resolution
    if (capture_frame(fd, cam_buffer, frame_size) != 0) {
        printf("Failed to capture low-resolution frame\n");
        return 1;
    }

/*
    // Simulate capturing a high-resolution frame on a zoom or click event
    printf("Capturing high-resolution frame...\n");
    if (capture_high_res_frame(fd, cam_buffer, frame_size) != 0) {
        printf("Failed to capture high-resolution frame\n");
        return 1;
    }

    // Switch back to low resolution for streaming
    printf("Switching back to low resolution...\n");
    if (switch_to_low_res(fd, cam_buffer, frame_size) != 0) {
        printf("Failed to switch back to low-resolution frame\n");
        return 1;
    }
*/
    // Start the server to stream low-res video
    start_server(PORT, cam_buffer, frame_size);

    // Clean up resources
    munmap(cam_buffer.start, cam_buffer.length);
    close(fd);

    return 0;
}

