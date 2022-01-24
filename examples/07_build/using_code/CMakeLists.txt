# This is an example showing how to use hsmcpp library as a source code

cmake_minimum_required(VERSION 2.6)
project(example)
set(BINARY_NAME "07_build_code")
set(CMAKE_CXX_STANDARD 11)

# ----------------------------------------------------------------------
# Configure and install HSMCPP library
SET(HSMBUILD_VERBOSE OFF CACHE BOOL "Disable HSM verbosity")
SET(HSMBUILD_EXAMPLES OFF CACHE BOOL "Disable HSM examples")
SET(HSMBUILD_TESTS OFF CACHE BOOL "Disable HSM tests")
SET(HSMBUILD_DISPATCHER_GLIB OFF CACHE BOOL "Disable Glib dispatcher")
SET(HSMBUILD_DISPATCHER_GLIBMM ON CACHE BOOL "Disable Glibmm dispatcher")
SET(HSMBUILD_DISPATCHER_QT OFF CACHE BOOL "Disable Qt dispatcher")
SET(HSMBUILD_DISPATCHER_STD ON CACHE BOOL "Enable STD dispatcher")
# ./hsmcpp folder should contain all files from hsmcpp repository
add_subdirectory(hsmcpp)

# ----------------------------------------------------------------------
# Application specific code. This is just an example it there are no restrictions to use it specifically in this way

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
target_include_directories(${BINARY_NAME} PUBLIC ${HSMCPP_STD_INCLUDE}
                                                 ${HSMCPP_GLIBMM_INCLUDE}
                                                 ${CMAKE_BINARY_DIR})
# Link with HSMCPP library
target_link_libraries(${BINARY_NAME} PUBLIC ${HSMCPP_STD_LIB} ${HSMCPP_GLIBMM_LIB})
# Set CXX flags for HSMCPP library
target_compile_options(${BINARY_NAME} PRIVATE ${HSMCPP_STD_CXX_FLAGS}
                                              ${HSMCPP_GLIBMM_CXX_FLAGS}
                                              ${HSM_DEFINITIONS_STD}
                                              ${HSM_DEFINITIONS_GLIBMM})