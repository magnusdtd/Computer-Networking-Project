#include <opencv2/opencv.hpp>
#include <ctime>
#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <windows.h>
#include <thread>

const std::string prefixFilePath = "./output-server/"; 

class VideoRecorder {
public:
    VideoRecorder(int frameRate = 10) : frameRate(frameRate), recording(false) {}

    void startRecording(int durationInSeconds);

private:
    int frameRate;
    bool recording;

    std::string getFileName();
};