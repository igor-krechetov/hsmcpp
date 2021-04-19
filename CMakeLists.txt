# HSM - Hierarchical State Machines

cmake_minimum_required(VERSION 2.6)
project(hsmcpp)

set(PROJECT_VERSION "1.0.0")
set(PROJECT_DESCRIPTION "C++ library that implements simple hierarchical state machines / finite state machines")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -fPIC")
set(CMAKE_VERBOSE_MAKEFILE OFF)

option(HSMBUILD_VERBOSE "Enable/disable HSM verbosity [feature for DEBUG]" OFF)
option(HSMBUILD_STRUCTURE_VALIDATION "Enable/disable HSM structure validation" ON)
option(HSMBUILD_THREAD_SAFETY "Enable/disable HSM thread safety" ON)
option(HSMBUILD_DISPATCHER_GLIB "Enable GLib dispatcher" ON)
option(HSMBUILD_DISPATCHER_GLIBMM "Enable GLibmm dispatcher" ON)
option(HSMBUILD_DISPATCHER_STD "Enable std::thread based dispatcher" ON)
option(HSMBUILD_DISPATCHER_QT "Enable Qt based dispatcher" ON)
option(HSMBUILD_TESTS "Build unittests" ON)
option(HSMBUILD_EXAMPLES "Build examples" ON)

message("--------------------------------------")
message("HSM build settings:\n")
message("HSMBUILD_VERBOSE = ${HSMBUILD_VERBOSE}")
message("HSMBUILD_STRUCTURE_VALIDATION = ${HSMBUILD_STRUCTURE_VALIDATION}")
message("HSMBUILD_THREAD_SAFETY = ${HSMBUILD_THREAD_SAFETY}")
message("HSMBUILD_DISPATCHER_GLIB = ${HSMBUILD_DISPATCHER_GLIB}")
message("HSMBUILD_DISPATCHER_GLIBMM = ${HSMBUILD_DISPATCHER_GLIBMM}")
message("HSMBUILD_DISPATCHER_STD = ${HSMBUILD_DISPATCHER_STD}")
message("HSMBUILD_DISPATCHER_QT = ${HSMBUILD_DISPATCHER_QT}")
message("HSMBUILD_TESTS = ${HSMBUILD_TESTS}")
message("HSMBUILD_EXAMPLES = ${HSMBUILD_EXAMPLES}")
message("--------------------------------------")

# ================================================
# Common modules
include(FindPkgConfig)

pkg_check_modules(GLIB REQUIRED glib-2.0)
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

set(GLIB_PKG_INCLUDE_DIRS
    ${GLIB_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
)

set(GLIB_PKG_LDFLAGS
    ${GLIB_LDFLAGS}
    ${GLIBMM_LDFLAGS}
)

# ================================================
# HSMCPP library
set(HSM_LIBRARY_NAME "hsmcpp")

set(HSM_DEFINITIONS_BASE "")

if (HSMBUILD_VERBOSE)
    set(HSM_DEFINITIONS_BASE "${HSM_DEFINITIONS_BASE} -DLOGGING_MODE_STRICT_VERBOSE")
else()
    set(HSM_DEFINITIONS_BASE "${HSM_DEFINITIONS_BASE} -DLOGGING_MODE_OFF")
endif()

if (HSMBUILD_STRUCTURE_VALIDATION)
    set(HSM_DEFINITIONS_BASE "${HSM_DEFINITIONS_BASE} -DHSM_ENABLE_SAFE_STRUCTURE")
endif()

if (NOT HSMBUILD_THREAD_SAFETY)
    set(HSM_DEFINITIONS_BASE "${HSM_DEFINITIONS_BASE} -DHSM_DISABLE_THREADSAFETY")
endif()

file (GLOB SRC_LIBRARY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherBase.cpp
)

add_definitions(${HSM_DEFINITIONS_BASE})
add_library(${HSM_LIBRARY_NAME} STATIC ${SRC_LIBRARY})
target_include_directories(${HSM_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ================================================
# DISPATCHER: GLIB
if (HSMBUILD_DISPATCHER_GLIB)
    set(HSM_DEFINITIONS_GLIB "-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB")
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB)
    add_library(${HSM_LIBRARY_NAME}_glib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLib.cpp)
    target_include_directories(${HSM_LIBRARY_NAME}_glib PUBLIC ${GLIB_PKG_INCLUDE_DIRS})
endif()

# ================================================
# DISPATCHER: GLIBMM
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(HSM_DEFINITIONS_GLIBMM "-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIBMM")
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIBMM)
    add_library(${HSM_LIBRARY_NAME}_glibmm STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLibmm.cpp)
    target_include_directories(${HSM_LIBRARY_NAME}_glibmm PUBLIC ${GLIB_PKG_INCLUDE_DIRS})
endif()

# ================================================
# DISPATCHER: STD
if (HSMBUILD_DISPATCHER_STD)
    set(HSM_DEFINITIONS_STD "-DHSM_BUILD_HSMBUILD_DISPATCHER_STD")
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_STD)
    add_library(${HSM_LIBRARY_NAME}_std STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherSTD.cpp)
endif()

# ================================================
# DISPATCHER: Qt
if (HSMBUILD_DISPATCHER_QT)
    # For Qt build to work make sure Qt5_DIR environment variable is set. For example:
    # export Qt5_DIR=/home/user/qt/5.13.2/gcc_64/lib/cmake/Qt5/

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    if(CMAKE_VERSION VERSION_LESS "3.7.0")
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()

    find_package(Qt5 COMPONENTS Core REQUIRED)

    set(HSM_DEFINITIONS_STD "-DDHSM_BUILD_HSMBUILD_DISPATCHER_QT")
    qt5_wrap_cpp(SRC_DISPATCHER_QT ${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/HsmEventDispatcherQt.hpp)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_QT)
    add_library(${HSM_LIBRARY_NAME}_qt STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherQt.cpp ${SRC_DISPATCHER_QT})
    target_include_directories(${HSM_LIBRARY_NAME}_qt PUBLIC ${Qt5Core_INCLUDE_DIRS})

    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)
endif()

# ================================================
# Export variables
set(HSMCPP_DEFINITIONS ${HSM_DEFINITIONS_BASE} CACHE STRING "" FORCE)
set(HSMCPP_CXX_FLAGS "-fPIC -pthread" CACHE STRING "" FORCE)

set(HSMCPP_LIB ${HSM_LIBRARY_NAME} CACHE STRING "" FORCE)
set(HSMCPP_STD_LIB ${HSM_LIBRARY_NAME}_std ${HSMCPP_LIB} CACHE STRING "" FORCE)
set(HSMCPP_GLIB_LIB ${HSM_LIBRARY_NAME}_glib ${HSMCPP_LIB} ${GLIB_PKG_LDFLAGS} CACHE STRING "" FORCE)
set(HSMCPP_GLIBMM_LIB ${HSM_LIBRARY_NAME}_glibmm ${HSMCPP_LIB} ${GLIB_PKG_LDFLAGS} CACHE STRING "" FORCE)
set(HSMCPP_QT_LIB ${HSM_LIBRARY_NAME}_qt ${HSMCPP_LIB} CACHE STRING "" FORCE)

set(HSMCPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "" FORCE)
set(HSMCPP_STD_INCLUDE ${HSMCPP_INCLUDE} CACHE STRING "" FORCE)
set(HSMCPP_GLIB_INCLUDE ${HSMCPP_INCLUDE} ${GLIB_PKG_INCLUDE_DIRS} CACHE STRING "" FORCE)
set(HSMCPP_GLIBMM_INCLUDE ${HSMCPP_INCLUDE} ${GLIB_PKG_INCLUDE_DIRS} CACHE STRING "" FORCE)
set(HSMCPP_QT_INCLUDE ${HSMCPP_INCLUDE} ${Qt5Core_INCLUDE_DIRS} CACHE STRING "" FORCE)

# ================================================
# Installation
set(HSM_INCLUDES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/")
set(CMAKE_INSTALL_INCLUDEDIR "include/hsmcpp")
set(CMAKE_INSTALL_LIBDIR "lib")

configure_file(./pkgconfig/hsmcpp.pc.in hsmcpp.pc @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/hsmcpp.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS ${HSM_LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${HSM_INCLUDES_ROOT}/hsm.hpp
              ${HSM_INCLUDES_ROOT}/HsmEventDispatcherBase.hpp
              ${HSM_INCLUDES_ROOT}/IHsmEventDispatcher.hpp
              ${HSM_INCLUDES_ROOT}/logging.hpp
              ${HSM_INCLUDES_ROOT}/variant.hpp
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if (HSMBUILD_DISPATCHER_STD)
    configure_file(./pkgconfig/hsmcpp_std.pc.in hsmcpp_std.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_std.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_std DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherSTD.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (HSMBUILD_DISPATCHER_GLIB)
    configure_file(./pkgconfig/hsmcpp_glib.pc.in hsmcpp_glib.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_glib.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_glib DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherGLib.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if (HSMBUILD_DISPATCHER_GLIBMM)
    configure_file(./pkgconfig/hsmcpp_glibmm.pc.in hsmcpp_glibmm.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_glibmm.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_glibmm DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherGLibmm.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

# ================================================
add_subdirectory(tools/scxml2gen)

if (HSMBUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (HSMBUILD_TESTS)
    add_subdirectory(tests)
endif()