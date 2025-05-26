#pragma once

#include <fstream>
#include <vector>
#include <audioclient.h>
#include <mmdeviceapi.h>

#include "AppConfig.h"

class AudioCapture {
private:
    IMMDevice* m_pDevice = nullptr;
    IAudioClient* m_pAudioClient = nullptr;
    IAudioCaptureClient* m_pCaptureClient = nullptr;
    WAVEFORMATEX* m_pWaveFormat = nullptr;
    std::ofstream m_outFile;
    AudioConfig m_config;
    UINT32 m_numFramesAvailable;

    void InitializeWASAPI();
    void CheckForError(HRESULT hr, const std::string& msg);
    std::vector<BYTE> ConvertChannels(std::vector<BYTE> vData, UINT32 numFrames, DWORD inputChannels, int targetChannels, DWORD bitsPerSample);
    std::vector<BYTE> ConvertStereoToMono(std::vector<BYTE> vData, UINT32 numFrames, DWORD bitsPerSample);
    std::vector<BYTE> ConvertMonoToStereo(std::vector<BYTE> vData, UINT32 numFrames, DWORD bitsPerSample);
    std::vector<BYTE> ConvertBitDepth(std::vector<BYTE> vData, UINT32 numFrames, WORD srcBitDepth, WORD dstBitDepth, int numChannels);

public:
    AudioCapture(const AudioConfig& cfg);
    ~AudioCapture();

    std::vector<BYTE> Capture();
};
