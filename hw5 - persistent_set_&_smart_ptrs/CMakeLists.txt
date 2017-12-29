cmake_minimum_required(VERSION 3.7)
project(persistent_set)

set(CMAKE_CXX_STANDARD 11)
include_directories(${persistent_set_SOURCE_DIR})

set(SOURCE_FILES persistent_set.h tests.cpp)
add_executable(persistent_set ${SOURCE_FILES}
                gtest/gtest-all.cc
                gtest/gtest.h
                gtest/gtest_main.cc)

target_link_libraries(${PROJECT_NAME} -lpthread)
