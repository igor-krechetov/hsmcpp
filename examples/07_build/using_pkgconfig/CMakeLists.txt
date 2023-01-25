cmake_minimum_required(VERSION 2.6)
project(07_build_pkgconfig)
set(BINARY_NAME "07_build_pkgconfig")
set(CMAKE_CXX_STANDARD 11)

find_package(PkgConfig REQUIRED)

# Configure HSMCPP library. Settings must be set before calling find_package
set(HSMCPP_CONFIG_VERBOSE ON CACHE BOOL "Enable internal HSM traces")
# set(HSMCPP_CONFIG_STRUCTURE_VALIDATION ON CACHE BOOL "Enable HSM structure validation")
# set(HSMCPP_CONFIG_THREAD_SAFETY ON CACHE BOOL "Enable HSM thread safety")
# set(HSMCPP_CONFIG_DEBUGGING ON CACHE BOOL "Enable HSM debugging")

# Include HSMCPP module with STD dispatcher (multiple can be specified)
# Possible component values: std, glib, glibmm, qt
find_package(hsmcpp COMPONENTS std glibmm)

# create folder for generated files
set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})
set(GEN_DIR_DIAGRAM ${CMAKE_BINARY_DIR}/gen_diagram)
file(MAKE_DIRECTORY ${GEN_DIR_DIAGRAM})

# generate HSM
generateHsm(GEN_07_HSM ../07_build.scxml "SwitchHsm" ${GEN_DIR} "GEN_OUT_SRC")
# generate state machine diagram (optional)
generateHsmDiagram(GEN_07_HSM_DIAGRAM ../07_build.scxml ${GEN_DIR_DIAGRAM}/07_build.plantuml)

# Define binary and dependencies
add_executable(${BINARY_NAME} ../main.cpp ${GEN_OUT_SRC})
add_dependencies(${BINARY_NAME} GEN_07_HSM)
add_dependencies(${BINARY_NAME} GEN_07_HSM_DIAGRAM)

# Add HSMCPP and GEN include directories
target_include_directories(${BINARY_NAME} PUBLIC ${HSMCPP_INCLUDE_DIRS} ${CMAKE_BINARY_DIR})
# Link with HSMCPP library
target_link_libraries(${BINARY_NAME} PUBLIC ${HSMCPP_LDFLAGS})
# Set defines for HSMCPP library
target_compile_options(${BINARY_NAME} PRIVATE ${HSMCPP_CFLAGS_OTHER})
