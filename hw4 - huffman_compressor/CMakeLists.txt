cmake_minimum_required(VERSION 2.8)
project(huffman_compressor)

include_directories(${huffman_compressor_SOURCE_DIR})

add_library(huffman_library
        init_.h
	huffman_lib.h
	Encoder.h
	Decoder.h
	FrequencyCounter.h
	Encoder.cpp
	Decoder.cpp
	FrequencyCounter.cpp)

install(TARGETS huffman_library DESTINATION lib)
install (FILES
        init_.h
	huffman_lib.h
	Encoder.h
	Decoder.h
	FrequencyCounter.h
        DESTINATION include/huffman_library)

add_executable(huffman_compressor
        main.cpp)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -Wall -std=c++11 -pedantic")
    #set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -fsanitize=address,undefined")
endif()
target_link_libraries(huffman_compressor huffman_library)