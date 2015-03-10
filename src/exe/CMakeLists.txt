cmake_minimum_required(VERSION 2.8)

set(PROJ_LIBS process utility)

# C++11 is ok for executables
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(execute main.cpp)
target_link_libraries(execute ${PROJ_LIBS} ${Boost_LIBRARIES})

add_executable(fdtee fdtee.cpp)

add_executable(pgraph pgraph.cpp)
target_link_libraries(pgraph ${PROJ_LIBS} ${Boost_LIBRARIES})
