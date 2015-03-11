cmake_minimum_required(VERSION 2.8)

project(streamgraph)

include(ExternalProject)

set(IGNORED_WARNINGS "-Wno-long-long -Wno-variadic-macros")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wall -pedantic ${IGNORED_WARNINGS}")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

find_package(Boost 1.40 REQUIRED COMPONENTS iostreams program_options system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

add_custom_target(deps ALL)

set(UNWIND_SRC ${CMAKE_BINARY_DIR}/vendor/unwind-src)
set(UNWIND_ROOT ${CMAKE_BINARY_DIR}/vendor/unwind)
ExternalProject_Add(
    unwind
    URL ${CMAKE_CURRENT_SOURCE_DIR}/vendor/libunwind-1.1
    SOURCE_DIR ${UNWIND_SRC}
    BINARY_DIR ${UNWIND_SRC}
    CONFIGURE_COMMAND ./configure --prefix=${UNWIND_ROOT}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
add_dependencies(deps unwind)
include_directories(${UNWIND_ROOT}/include)
set(UNWIND_LIBRARIES ${UNWIND_ROOT}/lib/libunwind.a lzma)


set(GLOG_SRC ${CMAKE_BINARY_DIR}/vendor/glog-src)
set(GLOG_ROOT ${CMAKE_BINARY_DIR}/vendor/glog)
ExternalProject_Add(
    glog
    URL ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glog-0.3.3
    DEPENDS unwind
    SOURCE_DIR ${GLOG_SRC}
    BINARY_DIR ${GLOG_SRC}
    CONFIGURE_COMMAND ./configure --prefix=${GLOG_ROOT}
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
add_dependencies(deps glog)
include_directories(${GLOG_ROOT}/include)
set(GLOG_LIBRARIES ${GLOG_ROOT}/lib/libglog.a ${UNWIND_LIBRARIES})



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
