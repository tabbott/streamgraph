cmake_minimum_required(VERSION 2.8)

set(LIB_SOURCES
    App.cpp
    App.hpp
    CommandBase.cpp
    CommandBase.hpp
    FdTeeCommand.cpp
    FdTeeCommand.hpp
    RunGraphCommand.cpp
    RunGraphCommand.hpp
)

add_library(ui ${LIB_SOURCES})
