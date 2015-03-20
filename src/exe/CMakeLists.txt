cmake_minimum_required(VERSION 2.8)

set(PROJ_LIBS ui process utility)

add_executable(streamgraph streamgraph.cpp)
target_link_libraries(streamgraph ${PROJ_LIBS} ${Boost_LIBRARIES} ${GLOG_LIBRARIES})

set_target_properties(streamgraph PROPERTIES OUTPUT_NAME streamgraph${EXE_VERSION_SUFFIX})
install(TARGETS streamgraph DESTINATION bin/)
