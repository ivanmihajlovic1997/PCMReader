#include "WAVFileConverter.h"


WAVFileConverter::WAVFileConverter(const std::string& path) : m_outputPath(path), m_outputFile(path, std::ios::binary)
{
    if (!m_outputFile.is_open())
        throw std::runtime_error("Failed to open output file: " + path);
}

WAVFileConverter::~WAVFileConverter()
{
    if (m_outputFile.is_open())
        m_outputFile.close();
}

void WAVFileConverter::Write(const std::vector<uint8_t>& audioData, const AudioConfig& format)
{
    writeWavHeader(format, audioData.size());
    m_outputFile.write(reinterpret_cast<const char*>(audioData.data()), audioData.size());
}

//TODO: find right types and use the template
void WAVFileConverter::writeWavHeader(const AudioConfig& format, int dataSize) {
    int byteRate = format.sampleRate * format.channels * format.targetBitDepth / 8;
    int blockAlign = format.channels * format.targetBitDepth / 8;
    int chunkSize = 36 + dataSize;

    m_outputFile.write("RIFF", 4);
    m_outputFile.write(reinterpret_cast<const char*>(&chunkSize), 4);
    m_outputFile.write("WAVE", 4);

    // fmt part
    m_outputFile.write("fmt ", 4);
    int subchunk1Size = 16;
    m_outputFile.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    m_outputFile.write(reinterpret_cast<const char*>(&format.audioFormat), 2);
    m_outputFile.write(reinterpret_cast<const char*>(&format.channels), 2);
    m_outputFile.write(reinterpret_cast<const char*>(&format.sampleRate), 4);
    m_outputFile.write(reinterpret_cast<const char*>(&byteRate), 4);
    m_outputFile.write(reinterpret_cast<const char*>(&blockAlign), 2);
    m_outputFile.write(reinterpret_cast<const char*>(&format.targetBitDepth), 2);

    // data part
    m_outputFile.write("data", 4);
    m_outputFile.write(reinterpret_cast<const char*>(&dataSize), 4);
}