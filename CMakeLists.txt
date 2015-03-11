cmake_minimum_required(VERSION 2.8)

project(streamgraph CXX)

include(ExternalProject)

set(IGNORED_WARNINGS "-Wno-long-long -Wno-variadic-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic ${IGNORED_WARNINGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Boost 1.40 REQUIRED COMPONENTS iostreams program_options system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

set(GLOG_SRC ${CMAKE_BINARY_DIR}/vendor/glog-src)
set(GLOG_ROOT ${CMAKE_BINARY_DIR}/vendor/glog)
add_custom_target(deps ALL)
ExternalProject_Add(
    glog
    URL ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glog-0.3.3
    SOURCE_DIR ${GLOG_SRC}
    BINARY_DIR ${GLOG_SRC}
    CONFIGURE_COMMAND ./configure --prefix=${GLOG_ROOT}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
add_dependencies(deps glog)

find_package(Threads REQUIRED)
link_libraries(${CMAKE_THREAD_LIBS_INIT})
# FIXME: check for libunwind
link_libraries(unwind)

include_directories(${GLOG_ROOT}/include)
set(GLOG_LIBRARY ${GLOG_ROOT}/lib/libglog.a)

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
