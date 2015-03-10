cmake_minimum_required(VERSION 2.8)

project(fdtee CXX)

set(IGNORED_WARNINGS "-Wno-long-long -Wno-variadic-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic ${IGNORED_WARNINGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Boost 1.40 REQUIRED COMPONENTS iostreams timer program_options system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(glog)

enable_testing(true)
add_subdirectory(vendor/gtest-1.7.0)
include_directories(vendor/gtest-1.7.0/include)

add_subdirectory(vendor/tinyxml2-2.2.0)
include_directories(vendor/tinyxml2-2.2.0)

include_directories(src/lib)

add_subdirectory(src/lib/process)
add_subdirectory(src/lib/utility)
add_subdirectory(src/exe)

add_subdirectory(test)
