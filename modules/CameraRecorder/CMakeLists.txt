cmake_minimum_required (VERSION 3.2)
project(CameraRecorder)

add_definitions(-O3)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

set(TARGET CameraRecorder)
set(PC_FFMPEG_INCLUDE_DIRS /usr/include/ffmpeg3/)
#include_directories(/usr/include/ffmpeg3/)

file(GLOB_RECURSE SOURCE_FILES "source/*.cpp")
set(MAIN_SOURCE_FILES ${SOURCE_FILES} "main.cpp")

find_package(Boost REQUIRED COMPONENTS system thread filesystem)
find_package(Threads REQUIRED)
find_package(FFmpeg
        COMPONENTS AVCODEC AVFORMAT AVUTIL AVDEVICE
        AVFILTER SWSCALE SWRESAMPLE REQUIRED)

add_executable(${TARGET} ${MAIN_SOURCE_FILES})

target_link_libraries(${TARGET}
    ${Boost_LIBRARIES}
    ${CMAKE_DL_LIBS}
    avcpp
    FFmpeg::FFmpeg
    Threads::Threads
    ssl crypto)
