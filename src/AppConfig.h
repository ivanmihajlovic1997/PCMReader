#pragma once
#include <string>

struct AudioConfig {
    std::string outputFile = "output.pcm";
    bool convertToMono = false;
    bool convertToStereo = false;
    int targetBitDepth = 32;
    int sampleRate = 48000;
    int channels = 1;
    int audioFormat = 3;
};

class AppConfig {
public:
	 static AudioConfig ParseArgs(int argc, char* argv[]);
};