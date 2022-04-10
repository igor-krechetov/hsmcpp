cmake_minimum_required(VERSION 2.6)
project(example)
set(BINARY_NAME "08_freertos")
set(CMAKE_CXX_STANDARD 11)

# ----------------------------------------------------------------------
# Configure FreeRTOS
SET(FREERTOS_ROOT "~/projects/FreeRTOS")
SET(FREERTOS_DIR "${FREERTOS_ROOT}/FreeRTOS")
SET(FREERTOS_PLUS_DIR "${FREERTOS_ROOT}/FreeRTOS-Plus")
SET(FREERTOS_KERNEL_DIR  "${FREERTOS_DIR}/Source")
SET(FREERTOS_LIB_DIR "${CMAKE_BINARY_DIR}/freertos")
SET(FREERTOS_PORT_LIB_DIR "${FREERTOS_LIB_DIR}/portable")
SET(FREERTOS_LIBS freertos_kernel freertos_kernel_port pthread)

SET(FREERTOS_LIB_DIRS ${FREERTOS_LIB_DIR} ${FREERTOS_PORT_LIB_DIR})
SET(FREERTOS_INCLUDE ${FREERTOS_KERNEL_DIR}/include
                     ${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/Posix
                     ${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils
                     ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include)

include(ExternalProject)
ExternalProject_Add(FREERTOS
                    SOURCE_DIR "${FREERTOS_KERNEL_DIR}"
                    BINARY_DIR "${FREERTOS_LIB_DIR}"
                    INSTALL_DIR ""
                    CMAKE_ARGS "-DFREERTOS_CONFIG_FILE_DIRECTORY=${CMAKE_CURRENT_SOURCE_DIR}"
                               "-DFREERTOS_HEAP=3"
                               "-DFREERTOS_PORT=GCC_POSIX"
                    INSTALL_COMMAND ""
                    )

# ----------------------------------------------------------------------
# Configure and install HSMCPP library
SET(HSMBUILD_VERBOSE OFF CACHE BOOL "Disable HSM verbosity")
SET(HSMBUILD_EXAMPLES OFF CACHE BOOL "Disable HSM examples")
SET(HSMBUILD_TESTS OFF CACHE BOOL "Disable HSM tests")
SET(HSMBUILD_DISPATCHER_GLIB OFF CACHE BOOL "Disable Glib dispatcher")
SET(HSMBUILD_DISPATCHER_GLIBMM OFF CACHE BOOL "Disable Glibmm dispatcher")
SET(HSMBUILD_DISPATCHER_QT OFF CACHE BOOL "Disable Qt dispatcher")
SET(HSMBUILD_DISPATCHER_STD OFF CACHE BOOL "Enable STD dispatcher")

SET(HSMBUILD_PLATFORM "FREERTOS" CACHE STRING "")
SET(HSMBUILD_FREERTOS_ROOT "${FREERTOS_ROOT}" CACHE STRING "")
SET(HSMBUILD_DISPATCHER_FREERTOS ON CACHE BOOL "Enable FreeRTOS based dispatcher")
SET(HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT ON CACHE BOOL "Enable default xPortIsInsideInterrupt")
SET(HSMBUILD_FREERTOS_CONFIG_FILE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" CACHE STRING "")

# ./hsmcpp folder should contain all files from hsmcpp repository
add_subdirectory(hsmcpp)

# ----------------------------------------------------------------------
# Application specific code. This is just an example it there are no restrictions to use it specifically in this way

# create folder for generated files
set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${GEN_DIR})

# add_definitions(-DHSM_LOGGING_MODE_STRICT_VERBOSE)

# generate HSM
generateHsm(GEN_08_HSM ./08_freertos.scxml "SwitchHsm" ${GEN_DIR} "GEN_OUT_SRC")

# Define binary and dependencies
add_executable(${BINARY_NAME} ./main.cpp
                              ./hooks.c
                              ${FREERTOS_SRC}
                              ${GEN_OUT_SRC})
add_dependencies(${BINARY_NAME} GEN_08_HSM)

target_link_directories(${BINARY_NAME} PUBLIC ${FREERTOS_LIB_DIRS})

# Add HSMCPP and GEN include directories
target_include_directories(${BINARY_NAME} PUBLIC ${FREERTOS_INCLUDE}
                                                 ${HSMCPP_FREERTOS_INCLUDE}
                                                 ${CMAKE_CURRENT_SOURCE_DIR}
                                                 ${CMAKE_BINARY_DIR})
# Link with HSMCPP library
target_link_libraries(${BINARY_NAME} PUBLIC ${HSMCPP_FREERTOS_LIB} ${FREERTOS_LIBS})
# Set CXX flags for HSMCPP library
target_compile_options(${BINARY_NAME} PRIVATE ${HSMCPP_FREERTOS_CXX_FLAGS})