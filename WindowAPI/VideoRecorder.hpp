#ifndef VIDEO_RECORDER_HPP
#define VIDEO_RECORDER_HPP

#include <thread>
#include <iostream>

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