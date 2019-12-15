cmake_minimum_required(VERSION 3.15)
project(FFmpegShim)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out)

add_definitions(-D__STDC_CONSTANT_MACROS)

link_directories(lib)

add_library(ffmpeg_shim SHARED decoder_shim.cpp rtsp_client.cpp util.cpp)
target_link_libraries(ffmpeg_shim avcodec avformat avutil)
target_include_directories(ffmpeg_shim PRIVATE include)
