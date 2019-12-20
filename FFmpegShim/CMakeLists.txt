cmake_minimum_required(VERSION 3.15)
project(FFmpegShim)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out)

add_definitions(-D__STDC_CONSTANT_MACROS)

link_directories(lib)

find_library(AVCODEC_LIBRARY avcodec HINTS lib)
find_library(AVFORMAT_LIBRARY avformat HINTS lib)
find_library(AVUTIL_LIBRARY avutil HINTS lib)
find_library(SWSCALE_LIBRARY swscale HINTS lib)

set(FFMPEG_LIBRARY ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})

add_library(ffmpeg_shim SHARED bmp_encoder.cpp decoder_shim.cpp rtsp_client.cpp scaler.cpp util.cpp)
target_link_libraries(ffmpeg_shim PRIVATE ${FFMPEG_LIBRARY})
target_include_directories(ffmpeg_shim PRIVATE include)

add_executable(scratch scratch.cpp)
target_link_libraries(scratch PRIVATE ${FFMPEG_LIBRARY})
target_include_directories(scratch PRIVATE include)
