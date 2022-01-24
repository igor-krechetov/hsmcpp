# Set default hsmcpp configuration
option(HSMCPP_CONFIG_VERBOSE "Enable/disable HSM verbosity" OFF)
option(HSMCPP_CONFIG_STRUCTURE_VALIDATION "Enable/disable HSM structure validation" ON)
option(HSMCPP_CONFIG_THREAD_SAFETY "Enable/disable HSM thread safety" ON)
option(HSMCPP_CONFIG_DEBUGGING "Enable/disable HSM debugging" ON)

message("-----------------------------")
message("HSMCPP configuration:")
message("-- HSMCPP_CONFIG_VERBOSE=${HSMCPP_CONFIG_VERBOSE}")
message("-- HSMCPP_CONFIG_STRUCTURE_VALIDATION=${HSMCPP_CONFIG_STRUCTURE_VALIDATION}")
message("-- HSMCPP_CONFIG_THREAD_SAFETY=${HSMCPP_CONFIG_THREAD_SAFETY}")
message("-- HSMCPP_CONFIG_DEBUGGING=${HSMCPP_CONFIG_DEBUGGING}")
message("-----------------------------")

set(HSMCPP_DEFINES "")

if (HSMCPP_CONFIG_VERBOSE)
    set(HSMCPP_DEFINES "${HSMCPP_DEFINES};-DHSM_LOGGING_MODE_STRICT_VERBOSE")
else()
    set(HSMCPP_DEFINES "${HSMCPP_DEFINES};-DHSM_LOGGING_MODE_OFF")
endif()

if (HSMCPP_CONFIG_STRUCTURE_VALIDATION)
    set(HSMCPP_DEFINES "${HSMCPP_DEFINES};-DHSM_ENABLE_SAFE_STRUCTURE")
endif()

if (NOT HSMCPP_CONFIG_THREAD_SAFETY)
    set(HSMCPP_DEFINES "${HSMCPP_DEFINES};-DHSM_DISABLE_THREADSAFETY")
endif()

if (HSMCPP_CONFIG_DEBUGGING)
    set(HSMCPP_DEFINES "${HSMCPP_DEFINES};-DHSMBUILD_DEBUGGING")
endif()

# load requested component
list(LENGTH hsmcpp_FIND_COMPONENTS COMPONETS_COUNT)
if (${COMPONETS_COUNT} LESS 1)
  message(FATAL_ERROR "Please specify at least one 1 component for hsmcpp module. Possible values are: std, glibmm, glib, qt")
endif()

set(HSMCPP_INCLUDE_DIRS "")
set(HSMCPP_LDFLAGS "")
set(HSMCPP_CFLAGS_OTHER "")

foreach(component ${hsmcpp_FIND_COMPONENTS})
  include(${CMAKE_CURRENT_LIST_DIR}/hsmcpp-${component}.cmake)
endforeach()

set(HSMCPP_CFLAGS_OTHER ${HSMCPP_DEFINES} ${HSMCPP_CFLAGS_OTHER})

include(${CMAKE_CURRENT_LIST_DIR}/scxml2gen/CMakeLists.txt)