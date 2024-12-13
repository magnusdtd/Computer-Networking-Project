#include "VideoRecorder.hpp"
#include <windows.h>
#include <dshow.h>
#include <string>
#include <chrono>
#include <thread>

std::string VideoRecorder::startRecording(int durationInSeconds, std::string &filePath)
{
    std::string result;
    auto start = std::chrono::steady_clock::now();

    // Initialize COM library
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        result = "Failed: Can't initialize COM library\n";
        std::cerr << result;
        return result;
    }

    // Create the filter graph manager
    IGraphBuilder *pGraph = NULL;
    hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
    if (FAILED(hr)) {
        result = "Failed: Can't create filter graph\n";
        std::cerr << result;
        CoUninitialize();
        return result;
    }

    // Create the capture graph builder
    ICaptureGraphBuilder2 *pBuilder = NULL;
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER, IID_ICaptureGraphBuilder2, (void **)&pBuilder);
    if (FAILED(hr)) {
        result = "Failed: Can't create capture graph builder\n";
        std::cerr << result;
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Set the filter graph to the capture graph builder
    pBuilder->SetFiltergraph(pGraph);

    // Create the system device enumerator
    ICreateDevEnum *pDevEnum = NULL;
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pDevEnum);
    if (FAILED(hr)) {
        result = "Failed: Can't create system device enumerator\n";
        std::cerr << result;
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Create the video input device enumerator
    IEnumMoniker *pEnum = NULL;
    hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);
    if (FAILED(hr) || pEnum == NULL) {
        result = "Failed: Can't create video input device enumerator\n";
        std::cerr << result;
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Get the first available video input device
    IMoniker *pMoniker = NULL;
    if (pEnum->Next(1, &pMoniker, NULL) != S_OK) {
        result = "Failed: Can't find video input device\n";
        std::cerr << result;
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Create the video capture filter
    IBaseFilter *pCap = NULL;
    hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter, (void **)&pCap);
    if (FAILED(hr)) {
        result = "Failed: Can't create video capture filter\n";
        std::cerr << result;
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Add the video capture filter to the filter graph
    hr = pGraph->AddFilter(pCap, L"Capture Filter");
    if (FAILED(hr)) {
        result = "Failed: Can't add video capture filter to filter graph\n";
        std::cerr << result;
        pCap->Release();
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Create the file writer filter
    IBaseFilter *pWriter = NULL;
    hr = pBuilder->SetOutputFileName(&MEDIASUBTYPE_Avi, std::wstring(filePath.begin(), filePath.end()).c_str(), &pWriter, NULL);
    if (FAILED(hr)) {
        result = "Failed: Can't create file writer filter\n";
        std::cerr << result;
        pCap->Release();
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Connect the video capture filter to the file writer filter
    hr = pBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, pCap, NULL, pWriter);
    if (FAILED(hr)) {
        result = "Failed: Can't connect video capture filter to file writer filter\n";
        std::cerr << result;
        pWriter->Release();
        pCap->Release();
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Start recording
    IMediaControl *pControl = NULL;
    hr = pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
    if (FAILED(hr)) {
        result = "Failed: Can't get media control interface\n";
        std::cerr << result;
        pWriter->Release();
        pCap->Release();
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    hr = pControl->Run();
    if (FAILED(hr)) {
        result = "Failed: Can't start recording\n";
        std::cerr << result;
        pControl->Release();
        pWriter->Release();
        pCap->Release();
        pMoniker->Release();
        pEnum->Release();
        pDevEnum->Release();
        pBuilder->Release();
        pGraph->Release();
        CoUninitialize();
        return result;
    }

    // Wait for the specified duration
    std::this_thread::sleep_for(std::chrono::seconds(durationInSeconds));

    // Stop recording
    pControl->Stop();

    // Clean up
    pControl->Release();
    pWriter->Release();
    pCap->Release();
    pMoniker->Release();
    pEnum->Release();
    pDevEnum->Release();
    pBuilder->Release();
    pGraph->Release();
    CoUninitialize();

    return "Screen recording started for " + std::to_string(durationInSeconds) + " seconds. File at: " + filePath;
}