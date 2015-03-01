cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    CStringArrayAdaptor.hpp
    FdMapping.cpp
    FdMapping.hpp
    FdSource.hpp
    ProcessManager.cpp
    ProcessManager.hpp
    ProcessSpec.hpp
)

add_library(procman ${LIB_SOURCES})

add_executable(fdtee main.cpp)

target_link_libraries(fdtee procman ${Boost_LIBRARIES})
