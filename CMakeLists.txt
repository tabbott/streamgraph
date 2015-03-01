cmake_minimum_required(VERSION 2.8)

project(fdtee CXX)

set(IGNORED_WARNINGS "-Wno-long-long -Wno-variadic-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -Wall -pedantic ${IGNORED_WARNINGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Boost 1.40 REQUIRED COMPONENTS iostreams timer program_options system)
include_directories(${Boost_INCLUDE_DIRS})

enable_testing(true)
add_subdirectory(vendor/gtest-1.7.0)
include_directories(vendor/gtest-1.7.0/include)

include_directories(src)
add_subdirectory(src)
add_subdirectory(test)
