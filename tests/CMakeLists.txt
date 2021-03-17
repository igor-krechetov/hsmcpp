set(TEST_BIN_NAME_TEMPLATE hsmUnitTests)

pkg_check_modules(GMOCK REQUIRED gmock)
pkg_check_modules(GTEST REQUIRED gtest)

# ================================================
# INCLUDES
include_directories(
PUBLIC
    ${GTEST_INCLUDE_DIRS}
    ${GMOCK_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
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
                         TestsCommon.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/ABCHsm.cpp
                         ${CMAKE_CURRENT_SOURCE_DIR}/hsm/AsyncHsm.cpp
)

# ================================================
# SOURCE CODE (Dispatcher: GLib)
set(TEST_BIN_GLIB ${TEST_BIN_NAME_TEMPLATE}GLib)

add_executable(${TEST_BIN_GLIB} mainGLib.cpp ${SRC_UNITTESTS_COMMON})
target_compile_definitions(${TEST_BIN_GLIB} PUBLIC -DTEST_HSM_GLIB)

# ================================================
# SOURCE CODE (Dispatcher: GLibmm)
set(TEST_BIN_GLIBMM ${TEST_BIN_NAME_TEMPLATE}GLibmm)

add_executable(${TEST_BIN_GLIBMM} mainGLibmm.cpp ${SRC_UNITTESTS_COMMON})
target_compile_definitions(${TEST_BIN_GLIBMM} PUBLIC -DTEST_HSM_GLIBMM)

# ================================================
# SOURCE CODE (Dispatcher: STD)
set(TEST_BIN_STD ${TEST_BIN_NAME_TEMPLATE}STD)

add_executable(${TEST_BIN_STD} mainSTD.cpp ${SRC_UNITTESTS_COMMON})
target_compile_definitions(${TEST_BIN_STD} PUBLIC -DTEST_HSM_STD)

# ================================================
# LIBRARIES
target_link_libraries(${TEST_BIN_GLIB}
PRIVATE
    ${HSM_LIBRARY_NAME}
PUBLIC
    ${GLIBMM_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
)

target_link_libraries(${TEST_BIN_GLIBMM}
PRIVATE
    ${HSM_LIBRARY_NAME}
PUBLIC
    ${GLIBMM_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
)

target_link_libraries(${TEST_BIN_STD}
PRIVATE
    ${HSM_LIBRARY_NAME}
PUBLIC
    ${GLIBMM_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
)
