cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    GraphXml.cpp
    GraphXml.hpp
    Process.cpp
    Process.hpp
    ProcessGraph.cpp
    ProcessGraph.hpp
    ProcessGroup.cpp
    ProcessGroup.hpp
    StatusXml.cpp
    StatusXml.hpp
)

add_library(process ${LIB_SOURCES})
