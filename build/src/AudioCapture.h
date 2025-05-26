#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstdint>
#include <audioclient.h>
#include <mmdeviceapi.h>
#include <comdef.h>
#include <initguid.h>
#include <windows.h>


struct AudioConfig {
    std::string outputFile = "output.pcm";
    bool convertToMono = false;
    bool convertToStereo = false;
    int targetBitDepth = 16;
};


class AudioCapture {
private:
    IMMDevice* pDevice = nullptr;
    IAudioClient* pAudioClient = nullptr;
    IAudioCaptureClient* pCaptureClient = nullptr;
    WAVEFORMATEX* pWaveFormat = nullptr;
    std::ofstream outFile;
    AudioConfig config;

    void InitializeWASAPI();
    void ConvertSampleDepthAndChannels(const uint8_t* buffer, UINT32 frameCount, std::vector<uint8_t>& outBuffer);

public:
    AudioCapture(const AudioConfig& cfg);
    ~AudioCapture();
    
    void Capture();
};
