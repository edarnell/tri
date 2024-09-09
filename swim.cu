#include <opencv2/opencv.hpp>
#include <iostream>

int main(int argc, char** argv) {
    // Open video file
    cv::VideoCapture cap("L6.AVI");

    if (!cap.isOpened()) {
        std::cout << "Error opening video stream or file" << std::endl;
        return -1;
    }

    cv::Mat frame, prevFrame, diffFrame, hsvFrame, mask, thresholdOutput, originalFrame, accumulatedMotion;
    cap.read(frame); // Read one frame to get frame size

    // Initialize accumulatedMotion to zero
    accumulatedMotion = cv::Mat::zeros(frame.size(), CV_8UC1);

    // Define a Region of Interest (ROI) to focus on the pool area
    cv::Rect poolROI(50, 100, frame.cols - 100, frame.rows - 200); // Adjust based on pool location

    while (cap.read(frame)) {
        // Copy the original frame for displaying with contours
        originalFrame = frame.clone();

        // Convert frame to HSV for color-based segmentation
        cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);

        // Apply refined color segmentation to detect skin tones and bright swimwear
        // Adjust the HSV range for swimmers' skin tones and costume colors
        cv::Scalar lowerColor = cv::Scalar(0, 30, 60);  // Example range for skin tones or costume color
        cv::Scalar upperColor = cv::Scalar(30, 255, 255);  // Narrow the range for better detection
        cv::inRange(hsvFrame, lowerColor, upperColor, mask);

        // Apply ROI to focus only on the pool area
        cv::Mat poolArea = mask(poolROI);

        // Motion Detection: Compare with the previous frame if available
        if (!prevFrame.empty()) {
            cv::absdiff(frame, prevFrame, diffFrame);

            // Convert diffFrame to grayscale for motion tracking
            cv::cvtColor(diffFrame, diffFrame, cv::COLOR_BGR2GRAY);

            // Threshold to detect motion (e.g., swimmers)
            cv::threshold(diffFrame, thresholdOutput, 25, 255, cv::THRESH_BINARY);

            // Accumulate motion over frames
            accumulatedMotion = accumulatedMotion + thresholdOutput;

            // Apply threshold to accumulated motion to avoid noise
            cv::threshold(accumulatedMotion, accumulatedMotion, 50, 255, cv::THRESH_BINARY);
        }

        // Find contours in the accumulated motion + color segmentation output
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(accumulatedMotion, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

        // Filter contours based on size to remove noise and small ripples
        for (size_t i = 0; i < contours.size(); i++) {
            double contourArea = cv::contourArea(contours[i]);

            // Assume swimmers are larger; remove small contours
            if (contourArea > 1000) {
                // Offset contour coordinates to match the original frame
                for (size_t j = 0; j < contours[i].size(); j++) {
                    contours[i][j].x += poolROI.x;
                    contours[i][j].y += poolROI.y;
                }

                // Draw contours on the original color frame
                cv::drawContours(originalFrame, contours, (int)i, cv::Scalar(0, 255, 0), 2);  // Green for swimmers
            }
        }

        // Display the original frame with contours drawn on it
        cv::imshow("Video", originalFrame);

        // Store the current frame as the previous frame for the next iteration
        prevFrame = frame.clone();

        // Press 'q' to quit
        if (cv::waitKey(30) >= 0) break;
    }

    return 0;
}


















