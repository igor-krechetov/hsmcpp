# HSM - Hierarchical State Machines

cmake_minimum_required(VERSION 3.16)
project(hsmcpp)

set(PROJECT_VERSION "0.24.4")
set(PROJECT_DESCRIPTION "C++ library that provides hierarchical state machines / finite state machines functionality")
set(CMAKE_VERBOSE_MAKEFILE OFF)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG(-std=c++17 COMPILER_SUPPORTS_CXX17)
CHECK_CXX_COMPILER_FLAG(-std=c++14 COMPILER_SUPPORTS_CXX14)
CHECK_CXX_COMPILER_FLAG(-std=c++11 COMPILER_SUPPORTS_CXX11)

if(COMPILER_SUPPORTS_CXX17)
    set(CMAKE_CXX_STANDARD 17)
    message("Using C++17")
elseif(COMPILER_SUPPORTS_CXX14)
    set(CMAKE_CXX_STANDARD 14)
    message("Using C++14")
elseif(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_STANDARD 11)
    message("Using C++11")
else()
    message(FATAL_ERROR "HSMCPP requires at least C++11 support. Please use a different C++ compiler.")
endif()

option(HSMBUILD_VERBOSE "Enable/disable HSM verbosity [feature for DEBUG]" OFF)
option(HSMBUILD_STRUCTURE_VALIDATION "Enable/disable HSM structure validation" ON)
option(HSMBUILD_THREAD_SAFETY "Enable/disable HSM thread safety" ON)
option(HSMBUILD_DEBUGGING "Enable/disable HSM debugging" ON)
option(HSMBUILD_DISPATCHER_GLIB "Enable GLib dispatcher" OFF)
option(HSMBUILD_DISPATCHER_GLIBMM "Enable GLibmm dispatcher" OFF)
option(HSMBUILD_DISPATCHER_STD "Enable std::thread based dispatcher" ON)
option(HSMBUILD_DISPATCHER_QT "Enable Qt based dispatcher" OFF)
option(HSMBUILD_TESTS "Build unittests" ON)
option(HSMBUILD_EXAMPLES "Build examples" ON)

set(HSMBUILD_PLATFORM "" CACHE STRING "Specifies platform")# Possible values: empty, FREERTOS
set(HSMBUILD_FREERTOS_ROOT "" CACHE STRING "FreeRTOS sources root folder")
set(HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/os/freertos" CACHE STRING "Path to folder with FreeRTOSConfig.h")
option(HSMBUILD_DISPATCHER_FREERTOS "Enable FreeRTOS based dispatcher" OFF)
option(HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT "Enable to add default xPortIsInsideInterrupt implementation" OFF)

message("--------------------------------------")
message("HSM build settings:\n")
message("HSMBUILD_VERBOSE = ${HSMBUILD_VERBOSE}")
message("HSMBUILD_STRUCTURE_VALIDATION = ${HSMBUILD_STRUCTURE_VALIDATION}")
message("HSMBUILD_THREAD_SAFETY = ${HSMBUILD_THREAD_SAFETY}")
message("HSMBUILD_DEBUGGING = ${HSMBUILD_DEBUGGING}")
message("HSMBUILD_DISPATCHER_GLIB = ${HSMBUILD_DISPATCHER_GLIB}")
message("HSMBUILD_DISPATCHER_GLIBMM = ${HSMBUILD_DISPATCHER_GLIBMM}")
message("HSMBUILD_DISPATCHER_STD = ${HSMBUILD_DISPATCHER_STD}")
message("HSMBUILD_DISPATCHER_QT = ${HSMBUILD_DISPATCHER_QT}")
message("HSMBUILD_DISPATCHER_FREERTOS = ${HSMBUILD_DISPATCHER_FREERTOS}")
message("HSMBUILD_TESTS = ${HSMBUILD_TESTS}")
message("HSMBUILD_EXAMPLES = ${HSMBUILD_EXAMPLES}")
message("")
message("HSMBUILD_PLATFORM = ${HSMBUILD_PLATFORM}")
message("HSMBUILD_FREERTOS_ROOT = ${HSMBUILD_FREERTOS_ROOT}")
message("HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY = ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY}")
message("HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT = ${HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT}")
message("--------------------------------------")

# ================================================================================================
# Common modules
if (NOT WIN32)
    include(FindPkgConfig)
endif()

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package (Threads REQUIRED)

if (HSMBUILD_DISPATCHER_GLIB)
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
endif()

add_subdirectory(tools/cmake)

# ================================================================================================
# HSMCPP library
set(HSM_LIBRARY_NAME "hsmcpp")

set(HSM_DEFINITIONS_BASE "")

if (HSMBUILD_VERBOSE)
    set(HSM_DEFINITIONS_BASE ${HSM_DEFINITIONS_BASE} -DHSM_LOGGING_MODE_STRICT_VERBOSE)
else()
    set(HSM_DEFINITIONS_BASE ${HSM_DEFINITIONS_BASE} -DHSM_LOGGING_MODE_OFF)
endif()

if (HSMBUILD_STRUCTURE_VALIDATION)
    set(HSM_DEFINITIONS_BASE ${HSM_DEFINITIONS_BASE} -DHSM_ENABLE_SAFE_STRUCTURE)
endif()

if (NOT HSMBUILD_THREAD_SAFETY)
    set(HSM_DEFINITIONS_BASE ${HSM_DEFINITIONS_BASE} -DHSM_DISABLE_THREADSAFETY)
endif()

if (HSMBUILD_DEBUGGING)
    set(HSM_DEFINITIONS_BASE ${HSM_DEFINITIONS_BASE} -DHSMBUILD_DEBUGGING)
endif()

file (GLOB SRC_LIBRARY
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hsm.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/variant.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherBase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/common/LockGuard.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/common/UniqueLock.cpp
)

# add platform specific code
if (HSMBUILD_PLATFORM STREQUAL "FREERTOS")
    SET (SRC_LIBRARY ${SRC_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/src/os/freertos/Mutex.cpp
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/freertos/ConditionVariable.cpp
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/freertos/CriticalSection.cpp
                                    ${CMAKE_CURRENT_SOURCE_DIR}/src/os/freertos/FreeRtosPort.cpp)
else()
    SET (SRC_LIBRARY ${SRC_LIBRARY} ${CMAKE_CURRENT_SOURCE_DIR}/src/os/stl/ConditionVariable.cpp)
endif()

add_definitions(${HSM_DEFINITIONS_BASE})
add_library(${HSM_LIBRARY_NAME} STATIC ${SRC_LIBRARY})
target_include_directories(${HSM_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if (NOT WIN32)
    target_compile_options(${HSM_LIBRARY_NAME} PRIVATE "-fPIC")
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# ================================================================================================
# Platform: FREERTOS
if (HSMBUILD_PLATFORM STREQUAL "FREERTOS")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/freertos-config.cmake)
    target_include_directories(${HSM_LIBRARY_NAME} PRIVATE ${FREERTOS_INCLUDE}
                                                           ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY})
endif()

# ================================================================================================
# Export variables
set(HSMCPP_DEFINITIONS ${HSM_DEFINITIONS_BASE} CACHE STRING "" FORCE)
if (NOT WIN32)
    set(HSMCPP_CXX_FLAGS "-fPIC" CACHE STRING "" FORCE)
else()
    set(HSMCPP_CXX_FLAGS "" CACHE STRING "" FORCE)
endif()

set(HSMCPP_LIB ${HSM_LIBRARY_NAME} CACHE STRING "" FORCE)
set(HSMCPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "" FORCE)

# ================================================================================================
# DISPATCHER: GLIB
if (HSMBUILD_DISPATCHER_GLIB)
    pkg_check_modules(GLIB REQUIRED glib-2.0)

    set(HSM_DEFINITIONS_GLIB ${HSM_DEFINITIONS_BASE} "-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB" CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB)
    add_library(${HSM_LIBRARY_NAME}_glib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLib.cpp)
    target_include_directories(${HSM_LIBRARY_NAME}_glib PUBLIC ${GLIB_INCLUDE_DIRS})
    target_compile_options(${HSM_LIBRARY_NAME}_glib PRIVATE "-fPIC")

    # Export variables
    set(HSMCPP_GLIB_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIB_LIB ${HSM_LIBRARY_NAME}_glib ${HSMCPP_LIB} ${GLIB_LDFLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIB_INCLUDE ${HSMCPP_INCLUDE} ${GLIB_INCLUDE_DIRS} CACHE STRING "" FORCE)
endif()

# ================================================================================================
# DISPATCHER: GLIBMM
if (HSMBUILD_DISPATCHER_GLIBMM)
    pkg_check_modules(GLIB REQUIRED glib-2.0)
    pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

    set(HSM_DEFINITIONS_GLIBMM ${HSM_DEFINITIONS_BASE} -DHSM_BUILD_HSMBUILD_DISPATCHER_GLIBMM CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIBMM)
    add_library(${HSM_LIBRARY_NAME}_glibmm STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLibmm.cpp)
    target_include_directories(${HSM_LIBRARY_NAME}_glibmm PUBLIC ${GLIB_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS})
    target_compile_options(${HSM_LIBRARY_NAME}_glibmm PRIVATE "-fPIC")

    # Export variables
    set(HSMCPP_GLIBMM_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIBMM_LIB ${HSM_LIBRARY_NAME}_glibmm ${HSMCPP_LIB} ${GLIB_LDFLAGS} ${GLIBMM_LDFLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIBMM_INCLUDE ${HSMCPP_INCLUDE} ${GLIB_INCLUDE_DIRS} ${GLIBMM_INCLUDE_DIRS} CACHE STRING "" FORCE)
endif()

# ================================================================================================
# DISPATCHER: STD
if (HSMBUILD_DISPATCHER_STD)
    set(HSM_DEFINITIONS_STD ${HSM_DEFINITIONS_BASE} -DHSM_BUILD_HSMBUILD_DISPATCHER_STD CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_STD)
    add_library(${HSM_LIBRARY_NAME}_std STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherSTD.cpp)
    target_compile_options(${HSM_LIBRARY_NAME}_std PRIVATE -fPIC)

    # Export variables
    if (NOT WIN32)
        set(HSMCPP_STD_CXX_FLAGS ${HSMCPP_CXX_FLAGS} -pthread CACHE STRING "" FORCE)
    else()
        set(HSMCPP_STD_CXX_FLAGS ${HSMCPP_CXX_FLAGS} "" CACHE STRING "" FORCE)
    endif()
    set(HSMCPP_STD_LIB ${HSM_LIBRARY_NAME}_std ${HSMCPP_LIB} ${CMAKE_THREAD_LIBS_INIT} CACHE STRING "" FORCE)
    set(HSMCPP_STD_INCLUDE ${HSMCPP_INCLUDE} CACHE STRING "" FORCE)
endif()

# ================================================================================================
# DISPATCHER: Qt
if (HSMBUILD_DISPATCHER_QT)
    # For Qt5 build make sure Qt5_DIR environment variable is set. For example:
    # export Qt5_DIR=/home/user/qt/5.13.2/gcc_64/lib/cmake/Qt5/

    # For Qt6 build use qt-cmake

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    if (WIN32)
        cmake_policy(SET CMP0071 NEW)
        cmake_policy(SET CMP0020 NEW)
    endif()

    if (CMAKE_VERSION VERSION_LESS "3.7.0")
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()

    # first check for Qt6
    find_package(Qt6 COMPONENTS Core)

    if (NOT Qt6_FOUND)
        if (NOT $ENV{Qt5_DIR} STREQUAL "")
            message("Using Qt5")
            find_package(Qt5 COMPONENTS Core REQUIRED)

            qt5_wrap_cpp(SRC_DISPATCHER_QT ${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/HsmEventDispatcherQt.hpp)
            SET(QtCore_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
            SET(QtCore_LIBRARIES ${Qt5Core_LIBRARIES})
        else()
            message( FATAL_ERROR "Qt not found")
        endif()
    else()
        message("Using Qt6")
        qt_standard_project_setup()# since Qt 6.3

        qt6_wrap_cpp(SRC_DISPATCHER_QT ${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/HsmEventDispatcherQt.hpp)
        SET(QtCore_INCLUDE_DIRS ${Qt6Core_INCLUDE_DIRS})
        SET(QtCore_LIBRARIES ${Qt6Core_LIBRARIES})
    endif()

    set(HSM_DEFINITIONS_QT ${HSM_DEFINITIONS_BASE} -DDHSM_BUILD_HSMBUILD_DISPATCHER_QT CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_QT)
    add_library(${HSM_LIBRARY_NAME}_qt STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherQt.cpp ${SRC_DISPATCHER_QT})
    target_include_directories(${HSM_LIBRARY_NAME}_qt PUBLIC ${QtCore_INCLUDE_DIRS})

    target_compile_options(${HSM_LIBRARY_NAME}_qt PRIVATE "-fPIC")

    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)

    # Export variables
    set(HSMCPP_QT_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_QT_INCLUDE ${HSMCPP_INCLUDE} ${QtCore_INCLUDE_DIRS} CACHE STRING "" FORCE)
    set(HSMCPP_QT_LIB ${HSM_LIBRARY_NAME}_qt ${HSMCPP_LIB} ${QtCore_LIBRARIES} CACHE STRING "" FORCE)

    registerQtWindeploy()
endif()

# ================================================================================================
# DISPATCHER: FreeRTOS
if (HSMBUILD_DISPATCHER_FREERTOS)
    set(HSM_DEFINITIONS_FREERTOS ${HSM_DEFINITIONS_BASE} -DHSM_BUILD_HSMBUILD_DISPATCHER_FREERTOS CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_FREERTOS)
    add_library(${HSM_LIBRARY_NAME}_freertos STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherFreeRTOS.cpp)
    target_compile_options(${HSM_LIBRARY_NAME}_freertos PRIVATE -fPIC)
    target_include_directories(${HSM_LIBRARY_NAME}_freertos PRIVATE ${FREERTOS_INCLUDE}
                                                                    ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY})

    # Export variables
    set(HSMCPP_FREERTOS_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_FREERTOS_LIB ${HSM_LIBRARY_NAME}_freertos ${HSMCPP_LIB} ${CMAKE_THREAD_LIBS_INIT} CACHE STRING "" FORCE)
    set(HSMCPP_FREERTOS_INCLUDE ${HSMCPP_INCLUDE} ${FREERTOS_INCLUDE} CACHE STRING "" FORCE)

    if (HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT)
        set(HSMCPP_FREERTOS_CXX_FLAGS ${HSMCPP_FREERTOS_CXX_FLAGS} -DBUILD_FREERTOS_DEFAULT_ISR_DETECT=1 CACHE STRING "" FORCE)
    endif()
endif()


# ================================================================================================
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
install(DIRECTORY ${HSM_INCLUDES_ROOT}/os
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

if (HSMBUILD_DISPATCHER_QT)
    configure_file(./pkgconfig/hsmcpp_qt.pc.in hsmcpp_qt.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_qt.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_qt DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherQt.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cmake/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME})
install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/CMakeLists.txt
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/scxml2gen.py
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/template.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/template.hpp
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/scxml2gen)

# ================================================================================================
add_subdirectory(tools/scxml2gen)

if (HSMBUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if (HSMBUILD_TESTS)
    add_subdirectory(tests)
endif()