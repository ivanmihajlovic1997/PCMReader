#pragma once
#include "IFileConverter.h"
#include <fstream>

class WAVFileConverter : public IFileConverter
{
public:
	WAVFileConverter(const std::string& path);
	~WAVFileConverter();
	void Write(const std::vector<uint8_t>& audioData, const AudioConfig& format) override;

private:
	void writeWavHeader(const AudioConfig& format, int dataSize);

	std::string m_outputPath;
	std::ofstream m_outputFile;

	template<typename T>
	void write_item(std::ostream& os, const T& item) {
		os.write(reinterpret_cast<const char*>(&item), sizeof T);
	}
};