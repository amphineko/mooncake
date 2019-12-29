cmake_minimum_required(VERSION 3.15)
project(lotus_shim)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ../../out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ../../out)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../../out)

add_definitions(-D__STDC_CONSTANT_MACROS)

find_library(AVCODEC_LIBRARY avcodec HINTS lib)
find_library(AVFORMAT_LIBRARY avformat HINTS lib)
find_library(AVUTIL_LIBRARY avutil HINTS lib)
find_library(SWSCALE_LIBRARY swscale HINTS lib)

set(FFMPEG_LIBRARY ${AVCODEC_LIBRARY} ${AVFORMAT_LIBRARY} ${AVUTIL_LIBRARY} ${SWSCALE_LIBRARY})

add_library(lotus_shim SHARED objects.cpp frame_encoder.cpp frame_reader.cpp frame_scaler.cpp stream.cpp)
target_link_libraries(lotus_shim PRIVATE ${FFMPEG_LIBRARY})
target_include_directories(lotus_shim PRIVATE include)

add_executable(stream_scratch stream_scratch.cpp)
target_link_libraries(stream_scratch PRIVATE ${FFMPEG_LIBRARY})
target_include_directories(stream_scratch PRIVATE include)
