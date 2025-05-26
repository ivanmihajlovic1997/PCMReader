#include "AudioCapture.h"

#include <iostream>
#include <string>
#include <stdexcept>
#include <cstdint>
#include <comdef.h>
#include <initguid.h>

AudioCapture::AudioCapture(const AudioConfig& cfg) : m_config(cfg) {
    m_outFile.open(m_config.outputFile, std::ios::binary);
    if (!m_outFile.is_open()) throw std::runtime_error("Failed to open output file: " + m_config.outputFile);
    InitializeWASAPI();
}

AudioCapture::~AudioCapture() {
    if (m_pCaptureClient)
    {
        m_pCaptureClient->Release();
        m_pCaptureClient = nullptr;
    }

    if (m_pAudioClient)
    {
        m_pAudioClient->Release();
        m_pAudioClient = nullptr;
    }
    if (m_pDevice)
    {
        m_pDevice->Release();
        m_pDevice = nullptr;
    }

    if (m_pWaveFormat)
        CoTaskMemFree(m_pWaveFormat);

    if (m_outFile.is_open())
        m_outFile.close();
}

void AudioCapture::InitializeWASAPI() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    CheckForError(hr, "Failed to create device enumerator");


    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &m_pDevice);
    pEnumerator->Release();
    CheckForError(hr, "Failed to get default audio endpoint");


    hr = m_pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&m_pAudioClient);
    CheckForError(hr, "Failed to activate audio client");

    //Retrieves the default audio format(e.g., sample rate, bit depth) for the audio stream.
    //Needed to configure the loopback capture with the correct format.
    /*
    wFormatTag      65534
    nChannels       2 // support for this to change
    nSamplesPerSec  48000
    nAvgBytesPerSec 384000
    nBlockAlign     8
    wBitsPerSample  32 //need support for this to change
    cbSize          22
    */
    hr = m_pAudioClient->GetMixFormat(&m_pWaveFormat);
    CheckForError(hr, "Failed to get mix format");

    hr = m_pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0, m_pWaveFormat, nullptr);
    CheckForError(hr, "Failed to initialize audio client");

    hr = m_pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&m_pCaptureClient);
    CheckForError(hr, "Failed to get capture client");
}

void AudioCapture::CheckForError(HRESULT hr, const std::string& msg)
{
    if (FAILED(hr)) 
        throw std::runtime_error(msg);
}

std::vector<BYTE> AudioCapture::ConvertChannels(std::vector<BYTE> vData, UINT32 numFrames, DWORD inputChannels, int targetChannels, DWORD bitsPerSample)
{
    std::vector<uint8_t> result = vData;
    
    if (inputChannels == 2 && targetChannels == 1) {
        result = ConvertStereoToMono(vData, numFrames, bitsPerSample);
    }

    if (inputChannels == 1 && targetChannels == 2) {
        result = ConvertMonoToStereo(vData, numFrames, bitsPerSample);
    }

    return result;
}

std::vector<BYTE> AudioCapture::ConvertStereoToMono(std::vector<BYTE> vData, UINT32 numFrames, DWORD bitsPerSample)
{
    std::vector<uint8_t> result;
    switch (bitsPerSample)
    {
    case 32: 
    {
        float* input = reinterpret_cast<float*>(vData.data());
        result.resize(numFrames * sizeof(float));
        float* output = reinterpret_cast<float*>(result.data());

        for (UINT32 i = 0; i < numFrames; ++i) {
            float left = input[2 * i];
            float right = input[2 * i + 1];
            output[i] = (left + right) * 0.5f;
        }
        break;
    }
    case 16: 
    {
        int16_t* input = reinterpret_cast<int16_t*>(vData.data());
        result.resize(numFrames * sizeof(int16_t));
        int16_t* output = reinterpret_cast<int16_t*>(result.data());

        for (UINT32 i = 0; i < numFrames; ++i) {
            int32_t left = input[2 * i];
            int32_t right = input[2 * i + 1];
            output[i] = static_cast<int16_t>((left + right) / 2);
        }
        break;
    }
    case 8:
    {
        int8_t* input = reinterpret_cast<int8_t*>(vData.data());
        result.resize(numFrames * sizeof(int8_t));
        int8_t* output = reinterpret_cast<int8_t*>(result.data());

        for (UINT32 i = 0; i < numFrames; ++i) {
            int16_t left = input[2 * i];
            int16_t right = input[2 * i + 1];
            output[i] = static_cast<int8_t>((left + right) / 2);
        }
        break;
    }
    default:
        return vData;
    }

    return result;
}

std::vector<BYTE> AudioCapture::ConvertMonoToStereo(std::vector<BYTE> vData, UINT32 numFrames, DWORD bitsPerSample)
{
    std::vector<BYTE> result;
    switch (bitsPerSample)
    {
    case 32:
    {
        float* input = reinterpret_cast<float*>(vData.data());
        result.resize(numFrames * 2 * sizeof(float));
        float* output = reinterpret_cast<float*>(result.data());

        for (UINT i = 0; i < numFrames; ++i)
        {
            output[2 * i] = input[i];
            output[2 * i + 1] = input[i];
        }
        break;
    }
    case 16:
    {
        int16_t* input = reinterpret_cast<int16_t*>(vData.data());
        result.resize(numFrames * 2 * sizeof(int16_t));
        int16_t* output = reinterpret_cast<int16_t*>(result.data());

        for (UINT32 i = 0; i < numFrames; ++i)
        {
            output[2 * i] = input[i];
            output[2 * i + 1] = input[i];
        }
        break;
    }
    case 8:
    {
        int8_t* input = reinterpret_cast<int8_t*>(vData.data());
        result.resize(numFrames * 2 * sizeof(int8_t));
        int8_t* output = reinterpret_cast<int8_t*>(result.data());

        for (UINT32 i = 0; i < numFrames; ++i)
        {
            output[2 * i] = input[i];
            output[2 * i + 1] = input[i];
        }
        break;
    }
    default:
        return vData;
    }

    return result;
}

std::vector<BYTE> AudioCapture::ConvertBitDepth(std::vector<BYTE> vData, UINT32 numFrames, WORD srcBitDepth, WORD dstBitDepth, int numChannels) {
    size_t totalSamples = numFrames * numChannels;
    size_t dstBytesPerSample = dstBitDepth / 8;

    std::vector<BYTE> result(totalSamples * dstBytesPerSample);

    for (size_t i = 0; i < totalSamples; ++i) {
        BYTE* inPtr = vData.data() + i * srcBitDepth/8;
        BYTE* outPtr = result.data() + i * dstBytesPerSample;
        //float case
        if (srcBitDepth == 32) {
            float floatSample = *reinterpret_cast<float*>(inPtr);
            if (floatSample < -1.0f)
                floatSample = -1.0f;
            if (floatSample > 1.0f)
                floatSample = 1.0f;

            switch (dstBitDepth) {
            case 8: {
                int8_t outSample = static_cast<int8_t>(floatSample * 127.0f);
                std::memcpy(outPtr, &outSample, 1);
                break;
            }
            case 16: {
                int16_t outSample = static_cast<int16_t>(floatSample * 32767.0f);
                std::memcpy(outPtr, &outSample, 2);
                break;
            }
            case 32: {
                int32_t outSample = static_cast<int32_t>(floatSample * 2147483647.0f);
                std::memcpy(outPtr, &outSample, 4);
                break;
            }
            default:
                return vData; // Unsupported destination bit depth
            }

            continue;
        }

        //int case
        int32_t sample = 0;
        switch (srcBitDepth) {
        case 8:
            sample = *reinterpret_cast<int8_t*>(inPtr);
            sample <<= 24;
            break;
        case 16:
            sample = *reinterpret_cast<int16_t*>(inPtr);
            sample <<= 16;
            break;
        case 32:
            sample = *reinterpret_cast<int32_t*>(inPtr);
            break;
        default:
            return vData; // Unsupported source bit depth
        }

        
        switch (dstBitDepth) {
        case 8: {
            int8_t outSample = static_cast<int8_t>(sample >> 24);
            std::memcpy(outPtr, &outSample, 1);
            break;
        }
        case 16: {
            int16_t outSample = static_cast<int16_t>(sample >> 16);
            std::memcpy(outPtr, &outSample, 2);
            break;
        }
        case 32: {
            int32_t outSample = sample;
            std::memcpy(outPtr, &outSample, 4);
            break;
        }
        default:
            return vData; // Unsupported destination bit depth
        }
    }

    return result;

}

std::vector<BYTE>  AudioCapture::Capture() {
    HRESULT hr = m_pAudioClient->Start();
    CheckForError(hr, "Failed to start audio client");

    UINT32 packetLength = 0;
    UINT32 numFramesAvailable;
    DWORD flags;
    BYTE* pData;
    std::vector<uint8_t> audioBuffer;

    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) { // Check for ESC key
        hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
        CheckForError(hr, "Failed to get packet size");

        while (packetLength != 0) {
            hr = m_pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            CheckForError(hr, "Failed to get buffer");
            
            std::vector<BYTE> buffer;
            buffer.resize(numFramesAvailable * m_pWaveFormat->nBlockAlign);
            std::copy(pData, pData + numFramesAvailable * m_pWaveFormat->nBlockAlign, buffer.begin());

            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {

                if (m_pWaveFormat->wBitsPerSample != m_config.targetBitDepth) {
                    buffer = ConvertBitDepth(buffer, numFramesAvailable, m_pWaveFormat->wBitsPerSample, m_config.targetBitDepth, m_pWaveFormat->nChannels);
                }

                if (m_config.convertToMono || m_config.convertToStereo) {
                    buffer = ConvertChannels(buffer, numFramesAvailable, m_pWaveFormat->nChannels, m_config.channels, m_config.targetBitDepth);
                }

                m_outFile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
                audioBuffer.insert(audioBuffer.end(), buffer.begin(), buffer.end());
            }

            hr = m_pCaptureClient->ReleaseBuffer(numFramesAvailable);
            CheckForError(hr, "Failed to release buffer");

            hr = m_pCaptureClient->GetNextPacketSize(&packetLength);
            CheckForError(hr, "Failed to get next packet size");
        }
        Sleep(10); // Prevent high CPU usage
    }

    hr = m_pAudioClient->Stop();
    CheckForError(hr, "Failed to start audio client");

    return audioBuffer;
}