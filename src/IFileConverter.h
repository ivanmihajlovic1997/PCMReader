#pragma once
#include <string>
#include <vector>
#include "AppConfig.h"

class IFileConverter {
public:
    virtual ~IFileConverter() = default;

    struct AudioFormat {
        int channels;
        int sampleRate;
        int bitsPerSample;
        enum class Encoding { PCM_INT16 = 1, IEEE_FLOAT32 = 3 } encoding;
    };

    virtual void Write(const std::vector<uint8_t>& audioData, const AudioConfig& format) = 0;
};