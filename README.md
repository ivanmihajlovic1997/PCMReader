# PCMReader

## Project Overview
PCMReader is a console application designed for capturing and converting audio data on a device. It uses the WASAPI (Windows Audio Session API) to capture audio and saves the raw audio data in a .pcm file. For playback convenience, the data is also saved in a .wav file. While support is currently limited to these two formats, the application is designed with extensibility in mind, allowing for future expansion to additional audio formats.

## Build and installation instructions
The project is built using CMake. To build it, run the runCmake.sh script, which will generate the necessary files in the ./build directory, including a .sln (Visual Studio Solution) file. Since the project was developed on Windows 11 using Visual Studio 2022, the following CMake command was used to generate the build:
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release && cmake --build build --config Release
If you are using a different version of Visual Studio, make sure to update the generator (-G) option accordingly. 

## Usage instructions with examples
The application uses command-line arguments to configure the format of the processed audio data. Below are some example recordings, made using audio from this video:
https://www.youtube.com/watch?v=YwNs1Z0qRY0

Example Commands and Outputs
./PCMReader.exe --channels 1 --format 3
→ Output: samples/outputStereoToMono32Bit.wav

./PCMReader.exe --bitdepth 16 --format 1
→ Output: samples/outputStereo16Bit.wav

./PCMReader.exe --bitdepth 8 --format 1
→ Output: samples/outputStereo8Bit.wav

./PCMReader.exe
→ Output: samples/outputStereo32Bit.wav (default settings)

Default Command-Line Arguments
--channels 1 (mono output)
--bitdepth 32
--format 3 (float)

## Explanation of design decisions
The application is designed to capture all audio data on the machine until the 'ESC' key is pressed. Upon startup, the AudioCapture component is initialized, which sets up WASAPI for audio capture. WASAPI returns a BYTE* pData pointer along with the number of captured frames. This data is stored in a buffer of type std::vector<BYTE> for easier manipulation—such as stereo-to-mono conversion and bit depth adjustment. Storing the data in a vector also simplifies operation validation and enables data injection, primarily for testing purposes.

## Description of testing approach
Unfortunately, I didn't manage to implement tests in time. The plan was to use the Catch2 testing framework to validate key functionality. The following three methods were intended as the primary focus of testing:

ConvertStereoToMono
ConvertMonoToStereo
ConvertBitDepth

To facilitate testing, these methods would be moved from private to protected access. A new test-specific class would then be created, inheriting publicly from the AudioCapture class. Each method could be tested by injecting a custom data stream and verifying the correctness of the output.
