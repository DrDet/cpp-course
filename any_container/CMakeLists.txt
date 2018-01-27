cmake_minimum_required(VERSION 3.7)
project(any_container)

set(CMAKE_CXX_STANDARD 11)
include_directories(${any_container_SOURCE_DIR})

set(SOURCE_FILES main.cpp any_iterator.h any_container.h)
add_executable(any_container ${SOURCE_FILES}
        gtest/gtest-all.cc
        gtest/gtest.h
        gtest/gtest_main.cc)
target_link_libraries(${PROJECT_NAME} -lpthread)