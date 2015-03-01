cmake_minimum_required(VERSION 2.8)

set(SOURCES
    TestFdMapping.cpp
    TestCStringArrayAdaptor.cpp
)

add_executable(TestFd ${SOURCES})
target_link_libraries(TestFd procman gtest_main gtest ${Boost_LIBRARIES})

add_test(NAME Fd COMMAND TestFd)
