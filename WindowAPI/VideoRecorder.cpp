#include "VideoRecorder.hpp"
#include <opencv2/opencv.hpp>

std::string VideoRecorder::startRecording(int durationInSeconds, std::string &filePath)
{
    std::string result;
    auto start = std::chrono::steady_clock::now();
    cv::VideoCapture cam(0);
    if (!cam.isOpened()) {
        result = "Failed: Can't open webcam\n";
        std::cerr << result;
        return result;
    }

    cv::Mat frame;
    cv::VideoWriter videoWriter;
    int frameInterval = 1000 / frameRate;

    while (true) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::seconds(durationInSeconds))
            break;

        cam >> frame;
        cv::flip(frame, frame, 1);

        if (!recording) {
            int frameWidth = static_cast<int>(cam.get(cv::CAP_PROP_FRAME_WIDTH));
            int frameHeight = static_cast<int>(cam.get(cv::CAP_PROP_FRAME_HEIGHT));
            videoWriter.open(filePath, cv::VideoWriter::fourcc('H', '2', '6', '4'), frameRate, cv::Size(frameWidth, frameHeight));
            if (!videoWriter.isOpened()) {
                result = "Failed to open video writer\n";
                std::cerr << result;
                videoWriter.release();
                return result;
            } else {
                recording = true;
            }
        }

        if (recording) {
            videoWriter.write(frame);
        }

        auto remainingTime = std::chrono::seconds(durationInSeconds) - elapsed;
        if (remainingTime > std::chrono::milliseconds(frameInterval)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(frameInterval));
        } else {
            break;
        }
    }

    cam.release();
    cv::destroyAllWindows();

    return "Screen recording started for " + std::to_string(durationInSeconds) + " seconds. File at: " + filePath;
}
