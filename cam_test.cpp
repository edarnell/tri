#include <opencv2/opencv.hpp>

int main() {
    cv::VideoCapture cap(0);  // Open /dev/video0
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        cv::imshow("Webcam", frame);
        if (cv::waitKey(30) >= 0) break;  // Exit on key press
    }

    cap.release();
    return 0;
}
