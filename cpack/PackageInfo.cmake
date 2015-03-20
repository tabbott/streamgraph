cmake_minimum_required(VERSION 2.8)

# .deb packaging
set(ARCH "i686")
if(${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    set(ARCH "x86_64")
endif ()

set(DEFAULT_ETC_ALTERNATIVES_PRIORITY 0)

# The format of the description field is a short summary line followed by a
# longer paragraph indented by a single space on each line
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Tool for creating graphs of streaming processes
 Streamgraph allows one to define a graph (in XML form)
 of processes to connect via pipes. In addition to straight
 pipelines (as in w | x | y | z), it also supports fanout
 (equivalent to something like w | tee >(x) >(y) | z in bash).")

set(CPACK_PACKAGE_NAME "streamgraph${EXE_VERSION_SUFFIX}")
set(CPACK_PACKAGE_VENDOR "TGI")
set(CPACK_PACKAGE_VERSION ${FULL_VERSION}${PACKAGE_VERSION_SUFFIX})

message(STATUS "EXE_VERSION_SUFFIX=${EXE_VERSION_SUFFIX}")
message(STATUS "FULL_VERSION=${FULL_VERSION}")
message(STATUS "package version=${CPACK_PACKAGE_VERSION}")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Travis Abbott <tgi.tabbott@gmail.com>")
set(CPACK_SYSTEM_NAME "Linux-${ARCH}")
set(CPACK_TOPLEVEL_TAG "Linux-${ARCH}")
set(CPACK_DEBIAN_PACKAGE_PROVIDES "streamgraph")
set(CPACK_DEBIAN_PACKAGE_SECTION utilities)
set(CPACK_DEBIAN_PACKAGE_PRIORITY optional)
set(CPACK_DEBIAN_PACKAGE_REPLACES "")
# looks like (aside from standard c/c++ stuff) the only dep might be
# liblzma.
# NOT SURE ABOUT MIN VERSIONS YET
set(CPACK_DEBIAN_PACKAGE_DEPENDS "")

configure_file(cpack/postinst.in cpack/postinst @ONLY)
configure_file(cpack/prerm.in cpack/prerm @ONLY)
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "cpack/postinst;cpack/prerm")

include(CPack)
