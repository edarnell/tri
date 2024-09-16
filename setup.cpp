#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <sstream>

// Serve file (e.g., HTML)
std::string serveFile(const std::string &fname, const std::string &ctype) {
    std::ifstream file(fname, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\n\nFile Not Found!";
    }

    std::stringstream res;
    res << "HTTP/1.1 200 OK\n";
    res << "Content-Type: " << ctype << "\n\n";
    res << file.rdbuf();
    file.close();

    return res.str();
}

// Serve snapshot image
void serveSnapshot(int camIdx, int sock) {
    cv::VideoCapture cam(camIdx, cv::CAP_V4L2);
    if (!cam.isOpened()) {
        std::cerr << "Failed to open camera " << camIdx << std::endl;
        std::string errRes = "HTTP/1.1 500 Internal Server Error\n\nFailed to open camera.";
        send(sock, errRes.c_str(), errRes.size(), 0);
        return;
    }

    cv::Mat frame;
    cam >> frame;  // Capture a frame
    cam.release();

    if (frame.empty()) {
        std::cerr << "Failed to capture snapshot from camera " << camIdx << std::endl;
        std::string errRes = "HTTP/1.1 500 Internal Server Error\n\nFailed to capture snapshot.";
        send(sock, errRes.c_str(), errRes.size(), 0);
        return;
    }

    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer);

    std::string res = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: " + std::to_string(buffer.size()) + "\n\n";
    send(sock, res.c_str(), res.size(), 0);
    send(sock, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0);
}

// Handle click requests
void handleClick(const std::string &req) {
    std::cout << "Received click request: " << req << std::endl;

    std::string cam = req.find("cam=left") != std::string::npos ? "left" : "right";
    size_t xPos = req.find("x=") + 2;
    size_t yPos = req.find("y=") + 2;

    try {
        int x = std::stoi(req.substr(xPos, req.find('&', xPos) - xPos));
        int y = std::stoi(req.substr(yPos, req.find(' ', yPos) - yPos));
        std::cout << "Click on " << cam << " camera at coordinates: X=" << x << " Y=" << y << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error parsing click coordinates: " << e.what() << std::endl;
    }
}

// Main HTTP server function
void startServer() {
    int serv_fd, new_sock;
    struct sockaddr_in addr;
    int addrlen = sizeof(addr);
    char buf[1024] = {0};

    // Check if cameras are available
    bool leftCamAvail = cv::VideoCapture(0).isOpened();  // Left camera
    bool rightCamAvail = cv::VideoCapture(2).isOpened();  // Right camera updated to /dev/video2

    // Create socket
    if ((serv_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind to port 8080
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(serv_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(serv_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server running on port 8080..." << std::endl;

    while (true) {
        if ((new_sock = accept(serv_fd, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Read HTTP request
        read(new_sock, buf, 1024);
        std::string req(buf);
        std::cout << "Received request: " << req << std::endl;

        // Serve the HTML file
        if (req.find("GET / ") != std::string::npos || req.find("GET /setup.html") != std::string::npos) {
            std::string res = serveFile("setup.html", "text/html");
            send(new_sock, res.c_str(), res.size(), 0);
        }
        // Serve left camera stream if available
        else if (req.find("/stream/left") != std::string::npos && leftCamAvail) {
            serveSnapshot(0, new_sock);  // Stream left camera
        }
        // Serve right camera stream if available
        else if (req.find("/stream/right") != std::string::npos && rightCamAvail) {
            serveSnapshot(2, new_sock);  // Stream right camera (/dev/video2)
        }
        // Serve snapshot requests
        else if (req.find("/snapshot?cam=left") != std::string::npos && leftCamAvail) {
            serveSnapshot(0, new_sock);  // Serve left camera snapshot
        }
        else if (req.find("/snapshot?cam=right") != std::string::npos && rightCamAvail) {
            serveSnapshot(2, new_sock);  // Serve right camera snapshot
        }
        // Handle click requests
        else if (req.find("/click") != std::string::npos) {
            handleClick(req);
            std::string clickRes = "HTTP/1.1 200 OK\n\nClick received.";
            send(new_sock, clickRes.c_str(), clickRes.size(), 0);
        }
        // Default 404 for other requests
        else {
            std::string res = "HTTP/1.1 404 Not Found\n\n";
            send(new_sock, res.c_str(), res.size(), 0);
        }

        close(new_sock);
    }
}

int main() {
    startServer();
    return 0;
}








