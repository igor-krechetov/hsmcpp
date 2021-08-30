set(TEST_BIN_NAME_TEMPLATE hsmUnitTests)

installGoogleTest()

# ================================================
# INCLUDES
include_directories(
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ================================================
# COMMON SOURCE CODE
set(SRC_UNITTESTS_COMMON 01_states.cpp
                         02_substates.cpp
                         03_transitions.cpp
                         04_multithreaded.cpp
                         05_callbacks.cpp
                         06_dispatchers.cpp
                         07_parallel.cpp
                         08_history.cpp
                         09_timers.cpp
                         20_variant.cpp
                         TestsCommon.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/ABCHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/AsyncHsm.cpp
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

    if (NOT WIN32)
        # this tool uses Linux specific mallinfo() API
        add_executable(test_memory_footprint test_memory_footprint.cpp)
        target_compile_definitions(test_memory_footprint PUBLIC -DTEST_HSM_STD)
        target_include_directories(test_memory_footprint PRIVATE ${HSMCPP_STD_INCLUDE})
        target_link_libraries(test_memory_footprint PRIVATE ${HSMCPP_STD_LIB})
        target_compile_options(test_memory_footprint PRIVATE ${HSMCPP_STD_CXX_FLAGS})
    endif()
endif()

# ================================================
# SOURCE CODE (Dispatcher: Qt)
if (HSMBUILD_DISPATCHER_QT)
    set(TEST_BIN_QT ${TEST_BIN_NAME_TEMPLATE}Qt)

    add_executable(${TEST_BIN_QT} mainQt.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_QT} PUBLIC -DTEST_HSM_QT)
    target_include_directories(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_INCLUDE})
    target_link_libraries(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_LIB} gmock_main)
    target_link_libraries(${TEST_BIN_QT} PUBLIC Qt5::Core)
    target_compile_options(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_CXX_FLAGS})
    deployQtApp(${TEST_BIN_QT})
endif()

# Used to compare binary size footprint (compared to 00_helloworld_std)
add_executable(emptyapp emptyapp.cpp)