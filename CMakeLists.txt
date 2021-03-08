# HSM - Hierarchical State Machines

cmake_minimum_required(VERSION 2.6)
project(hsmcpp)

set(PROJECT_VERSION "1.0.0")
set(PROJECT_DESCRIPTION "C++ library that implements simple hierarchical state machines / finite state machines")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
set(CMAKE_VERBOSE_MAKEFILE OFF)

option(VERBOSE "Enable/disable HSM verbosity [feature for DEBUG]" OFF)
option(STRUCTURE_VALIDATION "Enable/disable HSM structure validation" ON)
option(THREAD_SAFETY "Enable/disable HSM thread safety" ON)
option(DISPATCHER_GLIB "Enable GLib dispatcher" ON)
option(DISPATCHER_STD "Enable std::thread based dispatcher" ON)
option(BUILD_TESTS "Build unittets" ON)
option(BUILD_EXAMPLES "Build examples" ON)

message("--------------------------------------")
message("HSM build settings:\n")
message("VERBOSE = ${VERBOSE}")
message("STRUCTURE_VALIDATION = ${STRUCTURE_VALIDATION}")
message("THREAD_SAFETY = ${THREAD_SAFETY}")
message("DISPATCHER_GLIB = ${DISPATCHER_GLIB}")
message("DISPATCHER_STD = ${DISPATCHER_STD}")
message("BUILD_TESTS = ${BUILD_TESTS}")
message("BUILD_EXAMPLES = ${BUILD_EXAMPLES}")
message("--------------------------------------")

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

if (VERBOSE)
    add_definitions(-DLOGGING_MODE_STRICT_VERBOSE)
else()
    add_definitions(-DLOGGING_MODE_OFF)
endif()

if (STRUCTURE_VALIDATION)
    add_definitions(-DHSM_ENABLE_SAFE_STRUCTURE)
endif()

if (NOT THREAD_SAFETY)
    add_definitions(-DHSM_DISABLE_THREADSAFETY)
endif()

file (GLOB SRC_LIBRARY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherBase.cpp
)

if (DISPATCHER_GLIB)
    add_definitions(-DHSM_BUILD_DISPATCHER_GLIB)

    file (GLOB SRC_LIBRARY ${SRC_LIBRARY}
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLib.cpp)
endif()

if (DISPATCHER_STD)
    add_definitions(-DHSM_BUILD_DISPATCHER_STD)

    file (GLOB SRC_LIBRARY ${SRC_LIBRARY}
                           ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherSTD.cpp)
endif()

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
if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (BUILD_TESTS)
    add_subdirectory(tests)
endif()