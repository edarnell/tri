#include "camera.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <cstring>
#include <unistd.h>
#include <cstdio>

void handle_client(int client_fd, struct buffer &cam_buffer, size_t frame_size) {
    const char *header =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    send(client_fd, header, strlen(header), 0);

    while (true) {
        const char *boundary = "--frame\r\n";
        send(client_fd, boundary, strlen(boundary), 0);

        char frame_header[100];
        snprintf(frame_header, sizeof(frame_header),
                 "Content-Type: image/jpeg\r\n"
                 "Content-Length: %zu\r\n\r\n", frame_size);
        send(client_fd, frame_header, strlen(frame_header), 0);

        send(client_fd, cam_buffer.start, frame_size, 0);
        usleep(50000);  // 20 FPS
    }

    close(client_fd);
}

int start_server(int port, struct buffer &cam_buffer, size_t frame_size) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return -1;
    }

    listen(server_fd, 10);
    printf("Server running on port %d\n", port);

    while (true) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd >= 0) {
            handle_client(client_fd, cam_buffer, frame_size);
        }
    }

    close(server_fd);
    return 0;
}
