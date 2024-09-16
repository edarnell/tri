#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <opencv2/opencv.hpp> // For webcam handling
#include <sstream>

// Function to serve a file (e.g., HTML file)
std::string serveFile(const std::string& filename, const std::string& contentType) {
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return "HTTP/1.1 404 Not Found\n\nFile Not Found!";
    }

    std::stringstream response;
    response << "HTTP/1.1 200 OK\n";
    response << "Content-Type: " << contentType << "\n\n";
    response << file.rdbuf();
    file.close();

    return response.str();
}

// Serve a snapshot image
void serveSnapshot(int camIndex, int client_socket) {
    cv::VideoCapture cap(camIndex, cv::CAP_V4L2);
    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera " << camIndex << std::endl;
        std::string errorResponse = "HTTP/1.1 500 Internal Server Error\n\nFailed to open camera.";
        send(client_socket, errorResponse.c_str(), errorResponse.size(), 0);
        return;
    }

    cv::Mat frame;
    cap >> frame; // Capture a single frame
    cap.release();

    if (frame.empty()) {
        std::cerr << "Failed to capture snapshot from camera " << camIndex << std::endl;
        std::string errorResponse = "HTTP/1.1 500 Internal Server Error\n\nFailed to capture snapshot.";
        send(client_socket, errorResponse.c_str(), errorResponse.size(), 0);
        return;
    }

    std::vector<uchar> buffer;
    cv::imencode(".jpg", frame, buffer);

    std::string response = "HTTP/1.1 200 OK\nContent-Type: image/jpeg\nContent-Length: " + std::to_string(buffer.size()) + "\n\n";
    send(client_socket, response.c_str(), response.size(), 0);
    send(client_socket, reinterpret_cast<const char*>(buffer.data()), buffer.size(), 0);
}

// Handle click requests with debug output
void handleClick(const std::string& request) {
    std::cout << "Received click request: " << request << std::endl;

    std::string cam = request.find("cam=left") != std::string::npos ? "left" : "right";
    size_t xPos = request.find("x=") + 2;
    size_t yPos = request.find("y=") + 2;

    if (xPos == std::string::npos || yPos == std::string::npos) {
        std::cerr << "Error parsing click coordinates!" << std::endl;
        return;
    }

    try {
        int x = std::stoi(request.substr(xPos, request.find('&', xPos) - xPos));
        int y = std::stoi(request.substr(yPos, request.find(' ', yPos) - yPos));
        std::cout << "Click on " << cam << " camera at coordinates: X=" << x << " Y=" << y << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error converting coordinates: " << e.what() << std::endl;
    }
}

// Main HTTP server function with snapshot handling
void startServer() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};

    // Check if cameras are available
    bool leftCamAvailable = cv::VideoCapture(0).isOpened();
    bool rightCamAvailable = cv::VideoCapture(1).isOpened();

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "Socket creation error" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Bind to port 8080
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::cout << "Server running on port 8080..." << std::endl;

    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            std::cerr << "Accept failed" << std::endl;
            exit(EXIT_FAILURE);
        }

        // Read the HTTP request
        read(new_socket, buffer, 1024);
        std::string request(buffer);
        std::cout << "Received request: " << request << std::endl;

        // Serve the HTML file
        if (request.find("GET / ") != std::string::npos || request.find("GET /setup.html") != std::string::npos) {
            std::string response = serveFile("setup.html", "text/html");
            send(new_socket, response.c_str(), response.size(), 0);
        }
        // Serve left camera stream only if available
        else if (request.find("/stream/left") != std::string::npos && leftCamAvailable) {
            serveSnapshot(0, new_socket);  // Left camera snapshot
        }
        // Serve right camera stream only if available
        else if (request.find("/stream/right") != std::string::npos && rightCamAvailable) {
            serveSnapshot(1, new_socket);  // Right camera snapshot
        }
        // Handle click requests
        else if (request.find("/click") != std::string::npos) {
            handleClick(request);
            std::string clickResponse = "HTTP/1.1 200 OK\n\nClick received.";
            send(new_socket, clickResponse.c_str(), clickResponse.size(), 0);
        }
        // Default 404 for other requests
        else {
            std::string response = "HTTP/1.1 404 Not Found\n\n";
            send(new_socket, response.c_str(), response.size(), 0);
        }

        close(new_socket);
    }
}

int main() {
    startServer();
    return 0;
}






