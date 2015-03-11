cmake_minimum_required(VERSION 2.8)

set(PROJ_LIBS ui process utility)

add_executable(streamgraph streamgraph.cpp)
target_link_libraries(streamgraph ${PROJ_LIBS} ${Boost_LIBRARIES})
