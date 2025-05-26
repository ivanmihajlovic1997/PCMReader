#include <iostream>

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
#include <memory.h>

#include "AppConfig.h"
#include "AudioCapture.h"
#include "IFileConverter.h"
#include "WAVFileConverter.h"


int main(int argc, char** argv)
{
    try {
        HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (FAILED(hr)) throw std::runtime_error("COM initialization failed");
        AudioConfig config = AppConfig::ParseArgs(argc, argv);

        std::cout << "Capturing audio to " << config.outputFile << " until ESC is pressed\n";
        std::cout << "Mono: " << config.convertToMono << ", Stereo: " << config.convertToStereo
            << ", Bit Depth: " << config.targetBitDepth << "\n";

        AudioCapture capture(config);
        auto capturedData = capture.Capture();


        const char* outputWAV = "output.wav";
        
        std::unique_ptr<IFileConverter> WAVFile = std::make_unique<WAVFileConverter>(outputWAV);
        WAVFile->Write(capturedData, config);

        std::cout << "Capture stopped (ESC pressed)\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}