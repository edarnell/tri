#ifndef STREAM_SERVER_H
#define STREAM_SERVER_H

#include <cstddef>  // For size_t

struct buffer;

// Function declaration for starting the streaming server
int start_server(int port, struct buffer &cam_buffer, size_t frame_size);

#endif  // STREAM_SERVER_H
