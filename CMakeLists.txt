# HSM - Hierarchical State Machines

cmake_minimum_required(VERSION 3.16)
project(hsmcpp)

set(PROJECT_VERSION "0.36.0")
set(PROJECT_DESCRIPTION "C++ library for hierarchical state machines / finite state machines. Provides a code-free visual approach for defining state machine logic using GUI editors with automatic code and diagram generation. Check out https://hsmcpp.readthedocs.io for detailed documentation.")
set(CMAKE_VERBOSE_MAKEFILE OFF)

# break version into components
string(REPLACE "." ";" VERSION_LIST ${PROJECT_VERSION})
list(GET VERSION_LIST 0 PROJECT_VERSION_MAJOR)
list(GET VERSION_LIST 1 PROJECT_VERSION_MINOR)
list(GET VERSION_LIST 2 PROJECT_VERSION_MICRO)

# HSMCPP requires at least C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

set(HSMBUILD_TARGET "library" CACHE STRING "Desired build output. Possible values: library, platformio, arduinoide")

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
option(HSMBUILD_CODECOVERAGE "Build with code coverage" OFF)
option(HSMBUILD_CLANGTIDY "Enable validation with clang-tidy-15 (if available)" OFF)

set(HSMBUILD_PLATFORM "posix" CACHE STRING "Specifies target platform. Possible values: posix, windows, freertos, arduino")
set(HSMBUILD_FREERTOS_ROOT "" CACHE STRING "FreeRTOS sources root folder")
set(HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/os/freertos" CACHE STRING "Path to folder with FreeRTOSConfig.h")
option(HSMBUILD_DISPATCHER_FREERTOS "Enable FreeRTOS based dispatcher" OFF)
option(HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT "Enable to add default xPortIsInsideInterrupt implementation" OFF)

# convert everything to lovercase
string(TOLOWER ${HSMBUILD_TARGET} HSMBUILD_TARGET)
string(TOLOWER ${HSMBUILD_PLATFORM} HSMBUILD_PLATFORM)

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
message("HSMBUILD_CODECOVERAGE = ${HSMBUILD_CODECOVERAGE}")
message("")
message("HSMBUILD_PLATFORM = ${HSMBUILD_PLATFORM}")
if (HSMBUILD_PLATFORM STREQUAL "freertos")
    message("HSMBUILD_FREERTOS_ROOT = ${HSMBUILD_FREERTOS_ROOT}")
    message("HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY = ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY}")
    message("HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT = ${HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT}")
endif()
message("--------------------------------------")

# ================================================================================================
# Code Coverage support
if (HSMBUILD_CODECOVERAGE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/CodeCoverage.cmake)
    append_coverage_compiler_flags()
endif()

# ================================================================================================
# Common source code
set(HSM_PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(HSM_SRC_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/src/")
set(HSM_INCLUDES_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/")

configure_file(./include/hsmcpp/version.hpp.in version.hpp @ONLY)

set (LIBRARY_SRC ${HSM_SRC_ROOT}/hsm.cpp
                 ${HSM_SRC_ROOT}/HsmImpl.cpp
                 ${HSM_SRC_ROOT}/HsmImplTypes.cpp
                 ${HSM_SRC_ROOT}/variant.cpp
                 ${HSM_SRC_ROOT}/logging.cpp
                 ${HSM_SRC_ROOT}/HsmEventDispatcherBase.cpp
                 ${HSM_SRC_ROOT}/os/common/LockGuard.cpp
                 ${HSM_SRC_ROOT}/os/common/UniqueLock.cpp
                 ${HSM_SRC_ROOT}/os/common/CriticalSection.cpp)

set (LIBRARY_HEADERS ${HSM_INCLUDES_ROOT}/hsm.hpp
                     ${HSM_INCLUDES_ROOT}/HsmTypes.hpp
                     ${HSM_INCLUDES_ROOT}/HsmEventDispatcherBase.hpp
                     ${HSM_INCLUDES_ROOT}/IHsmEventDispatcher.hpp
                     ${HSM_INCLUDES_ROOT}/logging.hpp
                     ${HSM_INCLUDES_ROOT}/variant.hpp
                     ${HSM_INCLUDES_ROOT}/os/ConditionVariable.hpp
                     ${HSM_INCLUDES_ROOT}/os/CriticalSection.hpp
                     ${HSM_INCLUDES_ROOT}/os/InterruptsFreeSection.hpp
                     ${HSM_INCLUDES_ROOT}/os/LockGuard.hpp
                     ${HSM_INCLUDES_ROOT}/os/Mutex.hpp
                     ${HSM_INCLUDES_ROOT}/os/os.hpp
                     ${HSM_INCLUDES_ROOT}/os/UniqueLock.hpp
                     ${HSM_INCLUDES_ROOT}/os/common/LockGuard.hpp
                     ${HSM_INCLUDES_ROOT}/os/common/UniqueLock.hpp
                     ${HSM_INCLUDES_ROOT}/os/common/CriticalSection.hpp
                     ${HSM_INCLUDES_ROOT}/os/common/InterruptsFreeSection.hpp
                     ${CMAKE_BINARY_DIR}/version.hpp)

set(FILES_SCXML2GEN ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/scxml2gen.py
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/template.cpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/template.hpp
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/__init__.py
                    ${CMAKE_CURRENT_SOURCE_DIR}/tools/__init__.py)

# add platform specific files
if (HSMBUILD_PLATFORM STREQUAL "freertos")
    set (LIBRARY_SRC ${LIBRARY_SRC} ${HSM_SRC_ROOT}/os/freertos/Mutex.cpp
                                    ${HSM_SRC_ROOT}/os/freertos/ConditionVariable.cpp
                                    ${HSM_SRC_ROOT}/os/freertos/InterruptsFreeSection.cpp
                                    ${HSM_SRC_ROOT}/os/freertos/FreeRtosPort.cpp
                                    ${HSM_SRC_ROOT}/HsmEventDispatcherFreeRTOS.cpp)
    set (LIBRARY_HEADERS ${LIBRARY_HEADERS} ${HSM_INCLUDES_ROOT}/os/freertos/ConditionVariable.hpp
                                            ${HSM_INCLUDES_ROOT}/os/freertos/InterruptsFreeSection.hpp
                                            ${HSM_INCLUDES_ROOT}/os/freertos/FreeRTOSConfig.h
                                            ${HSM_INCLUDES_ROOT}/os/freertos/FreeRtosPort.hpp
                                            ${HSM_INCLUDES_ROOT}/os/freertos/Mutex.hpp
                                            ${HSM_INCLUDES_ROOT}/HsmEventDispatcherFreeRTOS.hpp)
elseif (HSMBUILD_PLATFORM STREQUAL "arduino")
    set (LIBRARY_SRC ${LIBRARY_SRC} ${HSM_SRC_ROOT}/os/arduino/ConditionVariable.cpp
                                    ${HSM_SRC_ROOT}/os/arduino/InterruptsFreeSection.cpp
                                    ${HSM_SRC_ROOT}/HsmEventDispatcherArduino.cpp)
    set (LIBRARY_HEADERS ${LIBRARY_HEADERS} ${HSM_INCLUDES_ROOT}/os/arduino/ConditionVariable.hpp
                                            ${HSM_INCLUDES_ROOT}/os/arduino/Mutex.hpp
                                            ${HSM_INCLUDES_ROOT}/HsmEventDispatcherArduino.hpp)
elseif (HSMBUILD_PLATFORM STREQUAL "posix")
    set (LIBRARY_SRC ${LIBRARY_SRC} ${HSM_SRC_ROOT}/os/posix/InterruptsFreeSection.cpp
                                    ${HSM_SRC_ROOT}/os/stl/ConditionVariable.cpp)
    set (LIBRARY_HEADERS ${LIBRARY_HEADERS} ${HSM_INCLUDES_ROOT}/os/posix/InterruptsFreeSection.hpp
                                            ${HSM_INCLUDES_ROOT}/os/stl/ConditionVariable.hpp
                                            ${HSM_INCLUDES_ROOT}/os/stl/Mutex.hpp)
elseif (HSMBUILD_PLATFORM STREQUAL "windows")
    set (LIBRARY_SRC ${LIBRARY_SRC} ${HSM_SRC_ROOT}/os/windows/InterruptsFreeSection.cpp
                                    ${HSM_SRC_ROOT}/os/stl/ConditionVariable.cpp)
    set (LIBRARY_HEADERS ${LIBRARY_HEADERS} ${HSM_INCLUDES_ROOT}/os/stl/ConditionVariable.hpp
                                            ${HSM_INCLUDES_ROOT}/os/stl/Mutex.hpp)
else()
    message(FATAL_ERROR "Unsupported HSMBUILD_PLATFORM=${HSMBUILD_PLATFORM}")
endif()

# ================================================================================================
# Build Target: package
set(DEPLOY_DIR_ROOT ${CMAKE_BINARY_DIR}/deploy)

if (HSMBUILD_TARGET STREQUAL "platformio")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target/package/platformio.cmake)
elseif (HSMBUILD_TARGET STREQUAL "arduinoide")
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target/package/arduinoide.cmake)
# ================================================================================================
# Build Target: library
elseif (HSMBUILD_TARGET STREQUAL "library")
    # =============================================
    # Common modules
    if (NOT WIN32)
        find_package(PkgConfig REQUIRED)
    endif()

    set(THREADS_PREFER_PTHREAD_FLAG TRUE)
    find_package (Threads REQUIRED)

    add_subdirectory(tools/cmake)
    add_subdirectory(tools/scxml2gen)

    # =============================================
    # Build HSMCPP library
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

    add_definitions(${HSM_DEFINITIONS_BASE})
    add_library(${HSM_LIBRARY_NAME} STATIC ${LIBRARY_SRC})
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_include_directories(${HSM_LIBRARY_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if (NOT WIN32)
        target_compile_options(${HSM_LIBRARY_NAME} PUBLIC "-fPIC")
    endif()

    # =============================================
    # Platform Environment
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/platforms/${HSMBUILD_PLATFORM}.cmake)

    if (HSMBUILD_PLATFORM STREQUAL "freertos")
        target_include_directories(${HSM_LIBRARY_NAME} PRIVATE ${FREERTOS_INCLUDE}
                                                               ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY})
    endif()

    # =============================================
    # Export variables
    set(HSMCPP_DEFINITIONS ${HSM_DEFINITIONS_BASE} CACHE STRING "" FORCE)
    if (NOT WIN32)
        set(HSMCPP_CXX_FLAGS "-fPIC" CACHE STRING "" FORCE)
    else()
        set(HSMCPP_CXX_FLAGS "" CACHE STRING "" FORCE)
    endif()

    set(HSMCPP_LIB ${HSM_LIBRARY_NAME} CACHE STRING "" FORCE)
    set(HSMCPP_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/include CACHE STRING "" FORCE)

    # =============================================
    # Build Dispatchers
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/dispatchers.cmake)

    # =============================================
    # Build Examples and Tests
    if (HSMBUILD_EXAMPLES)
        add_subdirectory(examples)
    endif()

    if (HSMBUILD_TESTS)
        add_subdirectory(tests)
    endif()

    # =============================================
    # Installation
    include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/target/library/library.cmake)

    # =============================================
    # Run clang-tidy
    if (HSMBUILD_CLANGTIDY)
        find_program(CLANG_TIDY_EXE NAMES "clang-tidy-15")

        if (CLANG_TIDY_EXE)
            string(CONCAT CLANG_TIDY_ARGS "-checks=-*,"
                                            "clang-analyzer-*,"
                                            "concurrency-*,"
                                            "cppcoreguidelines-*,"
                                            "performance-*,"
                                            "readability-*,"
                                            "modernize-*,"
                                            "-modernize-use-trailing-return-type,"
                                            "-readability-simplify-boolean-expr,"
                                            "-readability-identifier-length")

            # setup clang-tidy command
            set(CLANG_TIDY_COMMAND ${CLANG_TIDY_EXE} ${CLANG_TIDY_ARGS})

            # set CXX_CLANG_TIDY property after defining the target
            set_target_properties(${HSM_LIBRARY_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
        endif()
    endif()
# ================================================================================================
else()
    message(FATAL_ERROR "Unsupported HSMBUILD_TARGET=${HSMBUILD_TARGET} NEW")
endif()
