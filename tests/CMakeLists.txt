set(TEST_BIN_NAME_TEMPLATE hsmUnitTests)

pkg_check_modules(GMOCK REQUIRED gmock)
pkg_check_modules(GTEST REQUIRED gtest)

# ================================================
# INCLUDES
include_directories(
PUBLIC
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
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
    target_link_libraries(${TEST_BIN_GLIB} PRIVATE ${HSMCPP_GLIB_LIB} ${GMOCK_LDFLAGS})
endif()

# ================================================
# SOURCE CODE (Dispatcher: GLibmm)
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(TEST_BIN_GLIBMM ${TEST_BIN_NAME_TEMPLATE}GLibmm)

    add_executable(${TEST_BIN_GLIBMM} mainGLibmm.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_GLIBMM} PUBLIC -DTEST_HSM_GLIBMM)
    target_include_directories(${TEST_BIN_GLIBMM} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${TEST_BIN_GLIBMM} PRIVATE ${HSMCPP_GLIBMM_LIB} ${GMOCK_LDFLAGS})
endif()

# ================================================
# SOURCE CODE (Dispatcher: STD)
if (HSMBUILD_DISPATCHER_STD)
    set(TEST_BIN_STD ${TEST_BIN_NAME_TEMPLATE}STD)

    add_executable(${TEST_BIN_STD} mainSTD.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_STD} PUBLIC -DTEST_HSM_STD)
    target_include_directories(${TEST_BIN_STD} PRIVATE ${HSMCPP_STD_INCLUDE})
    target_link_libraries(${TEST_BIN_STD} PRIVATE ${HSMCPP_STD_LIB} ${GMOCK_LDFLAGS})
endif()

# ================================================
# SOURCE CODE (Dispatcher: Qt)
if (HSMBUILD_DISPATCHER_QT)
    set(TEST_BIN_QT ${TEST_BIN_NAME_TEMPLATE}Qt)

    add_executable(${TEST_BIN_QT} mainQt.cpp ${SRC_UNITTESTS_COMMON})
    target_compile_definitions(${TEST_BIN_QT} PUBLIC -DTEST_HSM_QT)
    target_include_directories(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_INCLUDE})
    target_link_libraries(${TEST_BIN_QT} PRIVATE ${HSMCPP_QT_LIB} ${GMOCK_LDFLAGS})
    target_link_libraries(${TEST_BIN_QT} PUBLIC Qt5::Core)
endif()

