cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    ChildFunction.hpp
    ChildProcess.cpp
    ChildProcess.hpp
    ChildProcessResult.hpp
    ExecWrapper.cpp
    ExecWrapper.hpp
    GraphXml.cpp
    GraphXml.hpp
    ProcessGraph.cpp
    ProcessGraph.hpp
    ProcessGroup.cpp
    ProcessGroup.hpp
    ProcessResult.hpp
    ProcessXml.hpp
    StatusXml.cpp
    StatusXml.hpp
)

add_library(process ${LIB_SOURCES})
