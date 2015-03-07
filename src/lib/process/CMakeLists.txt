cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    Process.cpp
    ProcessGraph.hpp
    ProcessGroup.cpp
    ProcessGroup.hpp
    Process.hpp
)

add_library(process ${LIB_SOURCES})
