cmake_minimum_required(VERSION 2.8)

project(streamgraph)

include(ExternalProject)

configure_file(test/TestConfig.hpp.in test/TestConfig.hpp @ONLY)
include_directories(${PROJECT_BINARY_DIR}/test)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cpack)

set(IGNORED_WARNINGS "-Wno-long-long -Wno-variadic-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic ${IGNORED_WARNINGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

include(BuildDeps)
include_directories(${Boost_INCLUDE_DIRS})

find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})

enable_testing(true)
add_subdirectory(vendor/gtest-1.7.0)
include_directories(vendor/gtest-1.7.0/include)

include_directories(vendor/rapidxml-1.13)

include_directories(src/lib)

add_subdirectory(src/lib/process)
add_subdirectory(src/lib/ui)
add_subdirectory(src/lib/utility)
add_subdirectory(src/exe)

add_subdirectory(test)

# Packaging
include(PackageInfo)
