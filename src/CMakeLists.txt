set(EXECUTABLE_OUTPUT_PATH ../bin)

add_executable(${PROJECT_NAME} 
	main.cpp 
	AppConfig.cpp
	AudioCapture.cpp
	WAVFileConverter.cpp
	IFileConverter.h
	AudioCapture.h
	AppConfig.h
	WAVFileConverter.h
	)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ole32
    avrt
    ksuser
)