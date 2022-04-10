# SET(FREERTOS_ROOT "~/projects/FreeRTOS")
SET(FREERTOS_DIR "${HSMBUILD_FREERTOS_ROOT}/FreeRTOS")
SET(FREERTOS_PLUS_DIR "${HSMBUILD_FREERTOS_ROOT}/FreeRTOS-Plus")
SET(FREERTOS_KERNEL_DIR  "${FREERTOS_DIR}/Source")
SET(FREERTOS_LIBS freertos_kernel freertos_kernel_port pthread)

SET(FREERTOS_INCLUDE ${FREERTOS_KERNEL_DIR}/include
                     ${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/Posix
                     ${FREERTOS_KERNEL_DIR}/portable/ThirdParty/GCC/Posix/utils
                     ${FREERTOS_PLUS_DIR}/Source/FreeRTOS-Plus-Trace/Include
                     ${CMAKE_SOURCE_DIR} CACHE STRING "")

add_definitions(-DPLATFORM_FREERTOS=1)

if (HSMBUILD_FREERTOS_DEFAULT_ISR_DETECT)
    add_definitions(-DBUILD_FREERTOS_DEFAULT_ISR_DETECT=1)
endif()