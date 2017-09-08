cmake_minimum_required(VERSION 3.7)
project(list_debug)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp list.h)
add_executable(list_debug ${SOURCE_FILES})