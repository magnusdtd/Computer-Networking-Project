#include "VideoRecorder.hpp"

void VideoRecorder::startRecording(int durationInSeconds)
{
    auto start = std::chrono::steady_clock::now();
    cv::VideoCapture cam(0);
    if (!cam.isOpened()) {
        std::cerr << "Can't open webcam\n";
        return;
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
            std::string fileName = getFileName();
            std::string filePath = prefixFilePath + fileName;
            int frameWidth = static_cast<int>(cam.get(cv::CAP_PROP_FRAME_WIDTH));
            int frameHeight = static_cast<int>(cam.get(cv::CAP_PROP_FRAME_HEIGHT));
            videoWriter.open(filePath, cv::VideoWriter::fourcc('H', '2', '6', '4'), frameRate, cv::Size(frameWidth, frameHeight));
            if (!videoWriter.isOpened()) {
                std::cerr << "Failed to open video writer\n";
                videoWriter.release();
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
}

std::string VideoRecorder::getFileName()
{
    std::time_t currentTime = std::time(0);
    std::tm localTime;
    localtime_s(&localTime, &currentTime);

    int year = localTime.tm_year + 1900;
    int month = localTime.tm_mon + 1;
    int day = localTime.tm_mday;
    int hour = localTime.tm_hour;
    int minute = localTime.tm_min;
    int seconds = localTime.tm_sec;

    std::ostringstream oss;
    oss << "VID" << year << (month < 10 ? "0" : "") << month
        << (day < 10 ? "0" : "") << day
        << (hour < 10 ? "0" : "") << hour
        << (minute < 10 ? "0" : "") << minute
        << (seconds < 10 ? "0" : "") << seconds
        << ".mp4";
    return oss.str();
}