cmake_minimum_required(VERSION 2.8)

add_custom_target(deps ALL)

set(UNWIND_SRC ${CMAKE_BINARY_DIR}/vendor/unwind-src)
set(UNWIND_ROOT ${CMAKE_BINARY_DIR}/vendor/unwind)
ExternalProject_Add(
    unwind
    URL ${CMAKE_SOURCE_DIR}/vendor/libunwind-1.1
    SOURCE_DIR ${UNWIND_SRC}
    BINARY_DIR ${UNWIND_SRC}
    CONFIGURE_COMMAND bash -ec "CXXFLAGS='${CMAKE_CXX_FLAGS}' ./configure --prefix=${UNWIND_ROOT}"
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
add_dependencies(deps unwind)
include_directories(${UNWIND_ROOT}/include)
set(UNWIND_LIBRARIES ${UNWIND_ROOT}/lib/libunwind.a lzma)

set(GLOG_SRC ${CMAKE_BINARY_DIR}/vendor/glog-src)
set(GLOG_ROOT ${CMAKE_BINARY_DIR}/vendor/glog)
ExternalProject_Add(
    glog
    URL ${CMAKE_SOURCE_DIR}/vendor/glog-0.3.3
    DEPENDS unwind
    SOURCE_DIR ${GLOG_SRC}
    BINARY_DIR ${GLOG_SRC}
    CONFIGURE_COMMAND ./configure --prefix=${GLOG_ROOT} --without-gflags
    BUILD_COMMAND make
    INSTALL_COMMAND make install
)
add_dependencies(deps glog)
include_directories(${GLOG_ROOT}/include)
set(GLOG_LIBRARIES ${GLOG_ROOT}/lib/libglog.a ${GFLAGS_LIBRARIES} ${UNWIND_LIBRARIES})

set(REQUIRED_BOOST_LIBS iostreams program_options system filesystem)

set(BOOST_ROOT ${CMAKE_BINARY_DIR}/vendor/boost)
set(BOOST_SRC ${CMAKE_BINARY_DIR}/vendor/boost-src)
set(BOOST_BUILD_LOG ${BOOST_SRC}/build.log)

foreach(libname ${REQUIRED_BOOST_LIBS})
    set(BOOST_BUILD_LIBS ${BOOST_BUILD_LIBS} --with-${libname})
    set(BOOST_LIBS ${BOOST_LIBS}
        ${BOOST_ROOT}/lib/${CMAKE_FIND_LIBRARY_PREFIXES}boost_${libname}${CMAKE_STATIC_LIBRARY_SUFFIX}
        )
endforeach(libname ${REQUIRED_BOOST_LIBS})

message(STATUS "Boost build log will be written to ${BOOST_BUILD_LOG}")
ExternalProject_Add(
    boost
    URL ${CMAKE_SOURCE_DIR}/vendor/boost-parts-1.57.0
    SOURCE_DIR ${BOOST_SRC}
    BINARY_DIR ${BOOST_SRC}
    CONFIGURE_COMMAND "./bootstrap.sh"
    BUILD_COMMAND
        ./b2 -d+2 --prefix=${BOOST_ROOT} --layout=system link=static threading=multi install
            cxxflags=${CMAKE_CXX_FLAGS}
            ${BOOST_BUILD_LIBS} ${BOOST_BUILD_OPTS} > ${BOOST_BUILD_LOG}
    INSTALL_COMMAND ""
)

add_dependencies(deps boost)

set(Boost_INCLUDE_DIRS ${BOOST_ROOT}/include)
set(Boost_LIBRARIES ${BOOST_LIBS})
