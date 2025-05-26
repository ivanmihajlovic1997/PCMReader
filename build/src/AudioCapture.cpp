#include "AudioCapture.h"

void AudioCapture::InitializeWASAPI() {
    HRESULT hr;
    IMMDeviceEnumerator* pEnumerator = nullptr;

    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) throw std::runtime_error("Failed to create device enumerator");

    hr = pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pDevice);
    pEnumerator->Release();
    if (FAILED(hr)) throw std::runtime_error("Failed to get default audio endpoint");

    hr = pDevice->Activate(__uuidof(IAudioClient), CLSCTX_ALL, nullptr, (void**)&pAudioClient);
    if (FAILED(hr)) throw std::runtime_error("Failed to activate audio client");

    hr = pAudioClient->GetMixFormat(&pWaveFormat);
    if (FAILED(hr)) throw std::runtime_error("Failed to get mix format");

    hr = pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,
        AUDCLNT_STREAMFLAGS_LOOPBACK,
        0, 0, pWaveFormat, nullptr);
    if (FAILED(hr)) throw std::runtime_error("Failed to initialize audio client");

    hr = pAudioClient->GetService(__uuidof(IAudioCaptureClient), (void**)&pCaptureClient);
    if (FAILED(hr)) throw std::runtime_error("Failed to get capture client");
}

void AudioCapture::ConvertSampleDepthAndChannels(const uint8_t* buffer, UINT32 frameCount, std::vector<uint8_t>& outBuffer) {
    int inputChannels = pWaveFormat->nChannels;
    int outputChannels = config.convertToMono ? 1 : (config.convertToStereo ? 2 : inputChannels);
    int inputBitDepth = pWaveFormat->wBitsPerSample;
    int outputBitDepth = config.targetBitDepth;

    outBuffer.clear();
    outBuffer.reserve(frameCount * outputChannels * (outputBitDepth / 8));

    for (UINT32 i = 0; i < frameCount; i++) {
        for (int ch = 0; ch < outputChannels; ch++) {
            int srcCh = config.convertToMono ? 0 : std::min(ch, inputChannels - 1);
            const uint8_t* sample = buffer + (i * inputChannels + srcCh) * (inputBitDepth / 8);

            if (inputBitDepth == outputBitDepth) {
                for (int b = 0; b < inputBitDepth / 8; b++) {
                    outBuffer.push_back(sample[b]);
                }
            }
            else if (inputBitDepth == 16 && outputBitDepth == 8) {
                int16_t sample16 = *(const int16_t*)sample;
                outBuffer.push_back(static_cast<uint8_t>(sample16 >> 8));
            }
            else if (inputBitDepth == 16 && outputBitDepth == 32) {
                int16_t sample16 = *(const int16_t*)sample;
                int32_t sample32 = static_cast<int32_t>(sample16) << 16;
                outBuffer.insert(outBuffer.end(), (uint8_t*)&sample32, (uint8_t*)&sample32 + 4);
            }
            else {
                throw std::runtime_error("Unsupported bit depth conversion");
            }
        }
    }
}

AudioCapture::AudioCapture(const AudioConfig& cfg) : config(cfg) {
    outFile.open(config.outputFile, std::ios::binary);
    if (!outFile.is_open()) throw std::runtime_error("Failed to open output file: " + config.outputFile);
    InitializeWASAPI();
}

AudioCapture::~AudioCapture() {
    if (pCaptureClient) pCaptureClient->Release();
    if (pAudioClient) pAudioClient->Release();
    if (pDevice) pDevice->Release();
    if (pWaveFormat) CoTaskMemFree(pWaveFormat);
    if (outFile.is_open()) outFile.close();
}

void AudioCapture::Capture() {
    HRESULT hr = pAudioClient->Start();
    if (FAILED(hr)) throw std::runtime_error("Failed to start audio client");

    UINT32 packetLength = 0;
    DWORD flags;
    BYTE* pData;
    UINT32 numFramesAvailable;
    std::vector<uint8_t> convertedBuffer;

    while (!(GetAsyncKeyState(VK_ESCAPE) & 0x8000)) { // Check for ESC key
        hr = pCaptureClient->GetNextPacketSize(&packetLength);
        if (FAILED(hr)) throw std::runtime_error("Failed to get packet size");

        while (packetLength != 0) {
            hr = pCaptureClient->GetBuffer(&pData, &numFramesAvailable, &flags, nullptr, nullptr);
            if (FAILED(hr)) throw std::runtime_error("Failed to get buffer");

            if (!(flags & AUDCLNT_BUFFERFLAGS_SILENT)) {
                ConvertSampleDepthAndChannels(pData, numFramesAvailable, convertedBuffer);
                outFile.write(reinterpret_cast<const char*>(convertedBuffer.data()), convertedBuffer.size());
            }

            hr = pCaptureClient->ReleaseBuffer(numFramesAvailable);
            if (FAILED(hr)) throw std::runtime_error("Failed to release buffer");

            hr = pCaptureClient->GetNextPacketSize(&packetLength);
            if (FAILED(hr)) throw std::runtime_error("Failed to get next packet size");
        }
        Sleep(10); // Prevent high CPU usage
    }

    pAudioClient->Stop();
}