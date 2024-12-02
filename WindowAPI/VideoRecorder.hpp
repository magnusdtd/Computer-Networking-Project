#ifndef VIDEO_RECORDER_HPP
#define VIDEO_RECORDER_HPP

#include <opencv2/opencv.hpp>
#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>
#include <thread>
#include <fstream>

const std::string prefixFilePath = "./output-server/"; 

class VideoRecorder {
public:
    VideoRecorder(int frameRate = 10) : frameRate(frameRate), recording(false) {}

    std::string startRecording(int durationInSeconds, std::string &filePath);

private:
    int frameRate;
    bool recording;
};

#endif