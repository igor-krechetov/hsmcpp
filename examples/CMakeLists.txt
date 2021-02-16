set(BINARY_NAME "01_trafficlight")

file (GLOB SRC_FILES 01_trafficlight.cpp)

add_executable(${BINARY_NAME} ${SRC_FILES})

target_include_directories(${BINARY_NAME}
PUBLIC
    ${GLIB_PKG_INCLUDE_DIRS}
PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${BINARY_NAME}
PUBLIC
    ${GLIB_PKG_LDFLAGS}
PRIVATE
    ${HSM_LIBRARY_NAME}
)

# pkg_check_modules(HSMCPP REQUIRED hsmcpp)

# file (GLOB SRC_FILES 01_trafficlight.cpp)

# add_executable(${BINARY_NAME} ${SRC_FILES})

# target_include_directories(${BINARY_NAME}
# PUBLIC
#     ${GLIB_PKG_INCLUDE_DIRS}
#     ${HSMCPP_INCLUDE_DIRS}
# PRIVATE
# )

# target_link_libraries(${BINARY_NAME}
# PUBLIC
#     ${GLIB_PKG_LDFLAGS}
#     ${HSMCPP_LDFLAGS}
# PRIVATE
# )