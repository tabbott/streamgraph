cmake_minimum_required(VERSION 2.8)

# C++11 is ok for test cases
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

set(SOURCES
    TestChildProcess.cpp
    TestProcess.cpp
    TestProcessGraph.cpp
    TestProcessGroup.cpp
)

add_executable(TestProcess ${SOURCES})
target_link_libraries(TestProcess process utility gtest_main_nolog gtest ${Boost_LIBRARIES} ${GLOG_LIBRARIES})
add_test(NAME Process COMMAND TestProcess)
