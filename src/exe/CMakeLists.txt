cmake_minimum_required(VERSION 2.8)

set(PROJ_LIBS process utility)

add_executable(execute main.cpp)
target_link_libraries(execute ${PROJ_LIBS} ${Boost_LIBRARIES})

add_executable(fdtee fdtee.cpp)

add_executable(pgraph pgraph.cpp)
target_link_libraries(pgraph ${PROJ_LIBS} ${Boost_LIBRARIES})
