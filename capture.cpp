#include <libcamera/libcamera.h>
#include <libcamera/camera_manager.h>
#include <libcamera/camera.h>
#include <libcamera/request.h>
#include <libcamera/stream.h>
#include <libcamera/framebuffer_allocator.h>
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <sys/mman.h>  // For mmap and munmap
#include <fcntl.h>     // For file controls like open

using namespace libcamera;

int main() {
    try {
        CameraManager manager;
        manager.start();

        std::cout << "Number of cameras detected: " << manager.cameras().size() << std::endl;

        if (manager.cameras().empty()) {
            std::cerr << "No cameras available!" << std::endl;
            manager.stop();
            return -1;
        }

        std::cout << "Attempting to get the first available camera." << std::endl;
        std::shared_ptr<Camera> camera = manager.get(0);

        if (!camera) {
            std::cerr << "Failed to get the camera!" << std::endl;
            manager.stop();
            return -1;
        }

        if (!camera->acquire()) {
            std::cerr << "Failed to acquire the camera!" << std::endl;
            manager.stop();
            return -1;
        }

        std::cout << "Camera acquired successfully." << std::endl;

        // Attempt to get the camera ID safely
        std::string cameraId = camera->id();
        if (cameraId.empty()) {
            std::cerr << "Camera ID is invalid!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        std::cout << "Camera ID: " << cameraId << std::endl;

        // Proceed with the rest of the configuration and capture logic...
        std::cout << "Generating camera configuration." << std::endl;
        std::unique_ptr<CameraConfiguration> cameraConfig = camera->generateConfiguration({StreamRole::Viewfinder});
        if (!cameraConfig) {
            std::cerr << "Failed to generate camera configuration!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        if (cameraConfig->validate() == CameraConfiguration::Status::Invalid) {
            std::cerr << "Invalid camera configuration!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        std::cout << "Configuring streams." << std::endl;
        StreamConfiguration &config = cameraConfig->at(0);
        config.size.width = 1920;
        config.size.height = 1080;
        config.pixelFormat = formats::MJPEG;

        int ret = camera->configure(cameraConfig.get());
        if (ret < 0) {
            std::cerr << "Failed to configure camera, error code: " << ret << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        Stream *stream = config.stream();
        std::cout << "Stream configured: width = " << config.size.width
                  << ", height = " << config.size.height << std::endl;

        FrameBufferAllocator allocator(camera);
        if (allocator.allocate(stream) < 0) {
            std::cerr << "Failed to allocate frame buffers!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        const std::vector<std::unique_ptr<FrameBuffer>> &buffers = allocator.buffers(stream);
        if (buffers.empty()) {
            std::cerr << "No buffers available!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        std::cout << "Creating request." << std::endl;
        std::unique_ptr<Request> request = camera->createRequest();
        if (!request) {
            std::cerr << "Failed to create request!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        FrameBuffer *buffer = buffers[0].get();
        request->addBuffer(stream, buffer);

        ret = camera->start();
        if (ret < 0) {
            std::cerr << "Failed to start camera!" << std::endl;
            camera->release();
            manager.stop();
            return -1;
        }

        std::cout << "Camera started." << std::endl;

        if (camera->queueRequest(request.get()) < 0) {
            std::cerr << "Failed to queue request!" << std::endl;
            camera->stop();
            camera->release();
            manager.stop();
            return -1;
        }

        std::cout << "Processing buffer." << std::endl;
        const FrameBuffer::Plane &plane = buffer->planes().front();
        int fd = plane.fd.get();
        void *data = mmap(NULL, plane.length, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            std::cerr << "Failed to map buffer memory!" << std::endl;
            camera->stop();
            camera->release();
            manager.stop();
            return -1;
        }

        std::ofstream file("capture.jpg", std::ios::binary);
        if (!file) {
            std::cerr << "Failed to open file for writing!" << std::endl;
            munmap(data, plane.length);
            camera->stop();
            camera->release();
            manager.stop();
            return -1;
        }

        file.write(reinterpret_cast<const char *>(data), plane.length);
        file.close();
        munmap(data, plane.length);

        std::cout << "Image saved to capture.jpg" << std::endl;

        camera->stop();
        camera->release();
        manager.stop();
    } catch (const std::exception &e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }

    return 0;
}
























