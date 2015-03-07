cmake_minimum_required(VERSION 2.8)

# C++11 is ok for test cases
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_library(gtest_main_nolog harness.cpp)

add_subdirectory(lib/process)
add_subdirectory(lib/utility)
