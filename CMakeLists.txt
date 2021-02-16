# HSM - Hierarchical State Machines

cmake_minimum_required(VERSION 2.6)
project(hsmcpp)

set(PROJECT_VERSION "1.0.0")
set(PROJECT_DESCRIPTION "C++ library that implements simple hierarchical state machines / finite state machines")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_VERBOSE_MAKEFILE OFF)

# add_definitions(-DLOGGING_MODE_STRICT_VERBOSE)
add_definitions(-DLOGGING_MODE_OFF)

# ================================================
# Common modules
include(FindPkgConfig)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

set (GLIB_PKG_INCLUDE_DIRS
    ${GLIB_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
)

set (GLIB_PKG_LDFLAGS
    ${GLIB_LDFLAGS}
    ${GLIBMM_LDFLAGS}
)

# ================================================
# HSMCPP library
set(HSM_LIBRARY_NAME "hsmcpp")

file (GLOB SRC_LIBRARY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variant.cpp
)

add_library(${HSM_LIBRARY_NAME} STATIC ${SRC_LIBRARY})

target_include_directories(${HSM_LIBRARY_NAME}
PUBLIC
    ${GLIB_PKG_INCLUDE_DIRS}
PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# ================================================
set(CMAKE_INSTALL_INCLUDEDIR "include/hsmcpp")
set(CMAKE_INSTALL_LIBDIR "lib")

configure_file(hsmcpp.pc.in hsmcpp.pc @ONLY)

install(TARGETS ${HSM_LIBRARY_NAME}
        DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(FILES "${PROJECT_BINARY_DIR}/hsmcpp.pc"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

# ================================================
add_subdirectory(examples)
add_subdirectory(tests)