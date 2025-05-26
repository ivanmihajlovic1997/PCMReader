#include "AppConfig.h"

AudioConfig AppConfig::ParseArgs(int argc, char* argv[]) {
    AudioConfig config;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--output" && i + 1 < argc) config.outputFile = argv[++i];
        else if (arg == "--mono") config.convertToMono = true;
        else if (arg == "--stereo") config.convertToStereo = true;
        else if (arg == "--bitdepth" && i + 1 < argc) config.targetBitDepth = std::stoi(argv[++i]);
        else if (arg == "--channels" && i + 1 < argc) config.channels = std::stoi(argv[++i]);
        else if (arg == "--format" && i + 1 < argc) config.audioFormat = std::stoi(argv[++i]);
    }
    return config;
}