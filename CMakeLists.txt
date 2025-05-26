cmake_minimum_required(VERSION 3.20)
project(PCMReader VERSION 1.0.0)

add_subdirectory(src)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    ole32
    avrt
    ksuser
)