set(TEST_BIN_NAME hsmUnitTests)

pkg_check_modules(GMOCK REQUIRED gmock)
pkg_check_modules(GTEST REQUIRED gtest)

# ================================================
# SOURCE CODE
set(SRC_UNITTESTS main.cpp
                  01_states.cpp
                  02_substates.cpp
                  03_transitions.cpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/hsm/TrafficLightHsm.cpp
)

add_executable(${TEST_BIN_NAME} ${SRC_UNITTESTS})

# ================================================
# INCLUDES
target_include_directories(${TEST_BIN_NAME}
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
# LIBRARIES
target_link_libraries(${TEST_BIN_NAME}
PUBLIC
    ${GLIB_LDFLAGS}
    ${GLIBMM_LDFLAGS}
    ${GTEST_LDFLAGS}
    ${GMOCK_LDFLAGS}
    rt
PRIVATE
    ${HSM_LIBRARY_NAME}
)
