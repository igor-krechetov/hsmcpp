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
# SOURCE CODE (Dispatcher: GLib)
set(TEST_BIN_GLIB ${TEST_BIN_NAME_TEMPLATE}GLib)

set(SRC_UNITTESTS_GLIB mainGLib.cpp
                       01_states.cpp
                       02_substates.cpp
                       03_transitions.cpp
                       04_multithreaded.cpp
                       05_callbacks.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/hsm/ABCHsm.cpp
                       ${CMAKE_CURRENT_SOURCE_DIR}/hsm/AsyncHsm.cpp
)

add_executable(${TEST_BIN_GLIB} ${SRC_UNITTESTS_GLIB})
target_compile_definitions(${TEST_BIN_GLIB} PUBLIC -DTEST_HSM_GLIB)

# ================================================
# SOURCE CODE (Dispatcher: STD)
set(TEST_BIN_STD ${TEST_BIN_NAME_TEMPLATE}STD)

set(SRC_UNITTESTS_STD mainSTD.cpp
                      01_states.cpp
                      02_substates.cpp
                      03_transitions.cpp
                      04_multithreaded.cpp
                      05_callbacks.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/hsm/ABCHsm.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/hsm/AsyncHsm.cpp
)

add_executable(${TEST_BIN_STD} ${SRC_UNITTESTS_STD})
target_compile_definitions(${TEST_BIN_STD} PUBLIC -DTEST_HSM_STD)

# ================================================
# LIBRARIES
target_link_libraries(${TEST_BIN_GLIB}
PUBLIC
    ${GLIB_LDFLAGS}
    ${GLIBMM_LDFLAGS}
    ${GTEST_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
PRIVATE
    ${HSM_LIBRARY_NAME}
)

target_link_libraries(${TEST_BIN_STD}
PUBLIC
    ${GLIB_LDFLAGS}
    ${GLIBMM_LDFLAGS}
    ${GTEST_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
PRIVATE
    ${HSM_LIBRARY_NAME}
)
