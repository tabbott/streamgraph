cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    CStringArrayAdaptor.hpp
    FdMapping.cpp
    FdMapping.hpp
    FdSource.cpp
    FdSource.hpp
    FdUtil.hpp
    io.hpp
    TempFile.cpp
    TempFile.hpp
    TimeUtil.hpp
)

add_library(utility ${LIB_SOURCES})
