set(TEST_BIN_NAME_TEMPLATE hsmUnitTests)

installGoogleTest()

# GoogleTest test discovery so `ctest` (and `just test`) can find and run the
# unit tests. Without this the test executables are built but never registered,
# so `ctest` reports "No tests were found". (enable_testing() is called at the
# top level in the root CMakeLists.txt.)
include(GoogleTest)

# Register a built GTest executable with CTest.
#
# Two registrations per binary:
#   1. gtest_discover_tests  -> one CTest entry per test case (normal `just test`
#      reporting, granular pass/fail).
#   2. add_test <target>_trace -> runs the WHOLE binary once with GoogleTest's
#      native XML output into ${GTEST_XML_DIR}/<target>.xml. This native XML
#      (unlike `ctest --output-junit`) preserves the TEST_REQUIREMENTS RecordProperty
#      entries that lobster-gtest needs for requirement traceability.
set(GTEST_XML_DIR ${CMAKE_BINARY_DIR}/gtest-xml)
function(registerUnitTest _target)
    # C++17 used for convenience. The library itself is built to the lower project standard.
    target_compile_features(${_target} PRIVATE cxx_std_17)
    gtest_discover_tests(${_target}
                         DISCOVERY_MODE PRE_TEST
                         DISCOVERY_TIMEOUT 60)
    add_test(NAME ${_target}_trace
             COMMAND ${_target} --gtest_output=xml:${GTEST_XML_DIR}/${_target}.xml)
endfunction()

# Check for memory leaks
# add_compile_options(-fsanitize=leak,address,undefined -fno-omit-frame-pointer -fno-common -O1)
# link_libraries(-fsanitize=leak,address,undefined)

# ================================================
# INCLUDES
include_directories(
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ================================================
# COMMON SOURCE CODE
set(SRC_UNITTESTS_COMMON ${CMAKE_CURRENT_SOURCE_DIR}/testcases/01_states.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/02_substates.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/03_transitions.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/04_multithreaded.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/05_callbacks.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/06_dispatchers.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/07_parallel.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/08_history.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/09_timers.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/10_state_actions.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/11_finalstate.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/12_debugging.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/13_public_api.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/20_variant.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/21_uniquelock.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/22_hsmimpltypes.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/testcases/99_regression_tests.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/TestsCommon.cpp

                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/ABCHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/BaseAsyncHsm.cpp

                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/gtestbadge/Badge.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/gtestbadge/BadgeEventListener.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/gtestbadge/BadgeTemplate.cpp

                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/RandomGenerators.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/ScopedEnv.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/ScopedFileGuard.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/utils/TempHsmLog.cpp
)

# ================================================
# SOURCE CODE (Dispatcher: GLib)
if (HSMBUILD_DISPATCHER_GLIB)
    set(TEST_BIN_GLIB ${TEST_BIN_NAME_TEMPLATE}GLib)

    add_executable(${TEST_BIN_GLIB} mainGLib.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_GLIB} PUBLIC -DTEST_HSM_GLIB)
    target_include_directories(${TEST_BIN_GLIB} PRIVATE ${HSMCPP_GLIB_INCLUDE})
    target_link_libraries(${TEST_BIN_GLIB} PRIVATE ${HSMCPP_GLIB_LIB} gmock_main)
    target_compile_options(${TEST_BIN_GLIB} PRIVATE ${HSMCPP_GLIB_CXX_FLAGS})
    registerUnitTest(${TEST_BIN_GLIB})
endif()

# ================================================
# SOURCE CODE (Dispatcher: GLibmm)
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(TEST_BIN_GLIBMM ${TEST_BIN_NAME_TEMPLATE}GLibmm)

    add_executable(${TEST_BIN_GLIBMM} mainGLibmm.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_GLIBMM} PUBLIC -DTEST_HSM_GLIBMM)
    target_include_directories(${TEST_BIN_GLIBMM} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${TEST_BIN_GLIBMM} PRIVATE ${HSMCPP_GLIBMM_LIB} gmock_main)
    target_compile_options(${TEST_BIN_GLIBMM} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
    registerUnitTest(${TEST_BIN_GLIBMM})
endif()

# ================================================
# SOURCE CODE (Dispatcher: STD)
if (HSMBUILD_DISPATCHER_STD)
    set(TEST_BIN_STD ${TEST_BIN_NAME_TEMPLATE}STD)

    add_executable(${TEST_BIN_STD} mainSTD.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_STD} PUBLIC -DTEST_HSM_STD)
    target_include_directories(${TEST_BIN_STD} PRIVATE ${HSMCPP_STD_INCLUDE})
    target_link_libraries(${TEST_BIN_STD} PRIVATE ${HSMCPP_STD_LIB} gmock_main)
    target_compile_options(${TEST_BIN_STD} PRIVATE ${HSMCPP_STD_CXX_FLAGS})
    registerUnitTest(${TEST_BIN_STD})

    if (NOT WIN32)
        # this tool uses Linux specific mallinfo() API and requires glib 2.33+
        include (CheckSymbolExists)

        CHECK_SYMBOL_EXISTS(mallinfo2 malloc.h MALLINFO2_EXISTS)

        if (MALLINFO2_EXISTS)
            add_executable(test_memory_footprint test_memory_footprint.cpp)
            target_compile_definitions(test_memory_footprint PUBLIC -DTEST_HSM_STD)
            target_include_directories(test_memory_footprint PRIVATE ${HSMCPP_STD_INCLUDE})
            target_link_libraries(test_memory_footprint PRIVATE ${HSMCPP_STD_LIB})
            target_compile_options(test_memory_footprint PRIVATE ${HSMCPP_STD_CXX_FLAGS})
        else()
            message("[SKIP] test_memory_footprint: mallinfo2 symbol not found (check glib version; version 2.33 or newer is required)")
        endif()
    endif()
endif()

# ================================================
# SOURCE CODE (Dispatcher: SingleThread -- driver-thread flavour, reuses the full suite)
# Built against the normal (thread-safety ON) core library. A harness-owned pump thread plays the
# role of the application loop so the blocking suite (transitionSync / BaseAsyncHsm rendezvous)
# can be reused verbatim; the test thread and the pump thread both touch the HSM, which is why
# this flavour needs the real synchronization primitives. It validates the single-threaded
# dispatcher's event/timer/queue logic. The no-op-sync ship configuration is covered by the
# INLINE flavour below, which compiles its own copy of the core with thread-safety OFF so BOTH
# suites build in a single pass (no separate HSMBUILD_THREAD_SAFETY=OFF configure needed).
if (HSMBUILD_DISPATCHER_SINGLETHREAD)
    set(TEST_BIN_SINGLETHREAD ${TEST_BIN_NAME_TEMPLATE}SingleThread)

    add_executable(${TEST_BIN_SINGLETHREAD} mainSingleThread.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_SINGLETHREAD} PUBLIC -DTEST_HSM_SINGLETHREAD)
    target_include_directories(${TEST_BIN_SINGLETHREAD} PRIVATE ${HSMCPP_SINGLETHREAD_INCLUDE})
    target_link_libraries(${TEST_BIN_SINGLETHREAD} PRIVATE ${HSMCPP_SINGLETHREAD_LIB} gmock_main)
    target_compile_options(${TEST_BIN_SINGLETHREAD} PRIVATE ${HSMCPP_SINGLETHREAD_CXX_FLAGS})
    # the harness pump thread needs pthreads even though the dispatcher itself does not
    find_package(Threads REQUIRED)
    target_link_libraries(${TEST_BIN_SINGLETHREAD} PRIVATE Threads::Threads)
    registerUnitTest(${TEST_BIN_SINGLETHREAD})
endif()

# ================================================
# SOURCE CODE (Dispatcher: SingleThread -- INLINE flavour, the real ship config)
# Exercises the ACTUAL shipping configuration of the single-threaded dispatcher: thread-safety
# OFF, no-op synchronization backend, no pump thread. Drives the dispatcher inline on a single
# thread via transition() + dispatchEvents().
#
# Thread-safety is a whole-translation-unit property (it selects the Mutex/AtomicFlag/CondVar
# backend used *inside* HsmImpl and the public headers), so this binary cannot link the tree's
# thread-safety-ON core library -- that would be an ODR violation. Instead we compile a
# TEST-INTERNAL copy of the exact core sources with a TARGET-SCOPED -DHSM_DISABLE_THREADSAFETY.
# This keeps the no-op build entirely inside the test target (it is never installed and never
# leaks onto any other target) and lets both single-threaded suites build in one pass.
if (HSMBUILD_DISPATCHER_SINGLETHREAD)
    # Test-internal no-op-sync core library: the same core translation units as the shipped
    # hsmcpp library (${LIBRARY_SRC}, propagated from the root scope) plus the single-threaded
    # dispatcher source, all compiled with HSM_DISABLE_THREADSAFETY.
    set(HSM_ST_NOTS_LIB hsmcpp_st_core_nothreadsafety)
    add_library(${HSM_ST_NOTS_LIB} STATIC
                ${LIBRARY_SRC}
                ${CMAKE_SOURCE_DIR}/src/HsmEventDispatcherSingleThread.cpp)
    # PUBLIC so the define also applies to the inline test TU that links this lib -- both must
    # resolve the os/ headers to the same (no-op) backend.
    target_compile_definitions(${HSM_ST_NOTS_LIB} PUBLIC -DHSM_DISABLE_THREADSAFETY)
    # mirror the base definitions and private include scoping the core library itself uses
    target_compile_definitions(${HSM_ST_NOTS_LIB} PRIVATE ${HSMCPP_DEFINITIONS})
    target_include_directories(${HSM_ST_NOTS_LIB} PUBLIC ${CMAKE_SOURCE_DIR}/include
                               PRIVATE ${CMAKE_SOURCE_DIR}/include/hsmcpp/os ${CMAKE_BINARY_DIR})
    if (NOT WIN32)
        target_compile_options(${HSM_ST_NOTS_LIB} PUBLIC "-fPIC")
    endif()

    set(TEST_BIN_SINGLETHREAD_INLINE ${TEST_BIN_NAME_TEMPLATE}SingleThreadInline)

    add_executable(${TEST_BIN_SINGLETHREAD_INLINE} singlethread_inline_tests.cpp)
    target_compile_definitions(${TEST_BIN_SINGLETHREAD_INLINE} PUBLIC -DTEST_HSM_SINGLETHREAD)
    target_link_libraries(${TEST_BIN_SINGLETHREAD_INLINE} PRIVATE ${HSM_ST_NOTS_LIB} gmock_main)
    registerUnitTest(${TEST_BIN_SINGLETHREAD_INLINE})
endif()

# ================================================
# SOURCE CODE (Dispatcher: Qt)
if (HSMBUILD_DISPATCHER_QT)
    set(TEST_BIN_QT ${TEST_BIN_NAME_TEMPLATE}Qt)

    add_executable(${TEST_BIN_QT} mainQt.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_QT} PUBLIC -DTEST_HSM_QT)
    target_include_directories(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_INCLUDE})
    target_link_libraries(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_LIB} gmock_main)
    target_compile_options(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_CXX_FLAGS})
    deployQtApp(${TEST_BIN_QT})
    registerUnitTest(${TEST_BIN_QT})
endif()

# ================================================
# SOURCE CODE (Dispatcher: FreeRTOS)
if (HSMBUILD_DISPATCHER_FREERTOS)
    # ----------------------------------------------------------------------
    # Configure FreeRTOS
    SET(FREERTOS_LIB_DIR "${CMAKE_BINARY_DIR}/freertos")
    SET(FREERTOS_PORT_LIB_DIR "${FREERTOS_LIB_DIR}/portable")
    SET(FREERTOS_LIB_DIRS ${FREERTOS_LIB_DIR} ${FREERTOS_PORT_LIB_DIR})
    SET(FREERTOS_LIBS freertos_kernel freertos_kernel_port pthread)

    include(ExternalProject)
    ExternalProject_Add(FREERTOS
                        SOURCE_DIR "${FREERTOS_KERNEL_DIR}"
                        BINARY_DIR "${FREERTOS_LIB_DIR}"
                        INSTALL_DIR ""
                        CMAKE_ARGS "-DFREERTOS_CONFIG_FILE_DIRECTORY=${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY}"
                                "-DFREERTOS_HEAP=3"
                                "-DFREERTOS_PORT=GCC_POSIX"
                        INSTALL_COMMAND ""
                        )

    set(TEST_BIN_FREERTOS ${TEST_BIN_NAME_TEMPLATE}FreeRTOS)

    add_executable(${TEST_BIN_FREERTOS} mainFreeRTOS.cpp ${FREERTOS_SRC} ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_FREERTOS} PUBLIC -DTEST_HSM_FREERTOS)
    target_include_directories(${TEST_BIN_FREERTOS} PRIVATE ${HSMCPP_FREERTOS_INCLUDE} ${HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY})
    target_link_directories(${TEST_BIN_FREERTOS} PUBLIC ${FREERTOS_LIB_DIRS})
    target_link_libraries(${TEST_BIN_FREERTOS} PRIVATE ${HSMCPP_FREERTOS_LIB} gmock_main ${FREERTOS_LIBS})
    target_compile_options(${TEST_BIN_FREERTOS} PRIVATE ${HSMCPP_FREERTOS_CXX_FLAGS})
endif()

# ================================================
# INTERNAL INCLUDE SCOPING
# 22_hsmimpltypes.cpp needs the internal src/HsmImplTypes.hpp (not part of the
# public include tree). Scope the private src/ include path to THAT one source
# file via a SOURCE property so it applies to every dispatcher binary that
# compiles it, without leaking src/ onto any other test translation unit or any
# production target. Must come after the executables are defined so the source
# file is already part of their build. Fallback if a generator ever rejects the
# SOURCE property across the multi-target layout:
#   foreach(_bin IN LISTS ... ) target_include_directories(${_bin} PRIVATE ${CMAKE_SOURCE_DIR}/src) endforeach()
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/testcases/22_hsmimpltypes.cpp
             APPEND PROPERTY INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src)

# ================================================
# Used to compare binary size footprint (compared to 00_helloworld_std)
add_executable(emptyapp emptyapp.cpp)