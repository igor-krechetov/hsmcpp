if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_05 "05_timers")
    set(BINARY_NAME_05_GEN "05_timers_gen")

    set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
    file(MAKE_DIRECTORY ${GEN_DIR})

    add_executable(${BINARY_NAME_05} 05_timers.cpp)
    target_include_directories(${BINARY_NAME_05} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${BINARY_NAME_05} PRIVATE ${HSMCPP_GLIBMM_LIB})
    target_compile_options(${BINARY_NAME_05} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})

    generateHsm(GEN_05_HSM ./05_timers.scxml "TimerHsm" ${GEN_DIR} "GEN_OUT_SRC_TIMERS")
    add_executable(${BINARY_NAME_05_GEN} 05_timers_generated.cpp ${GEN_OUT_SRC_TIMERS})
    add_dependencies(${BINARY_NAME_05_GEN} GEN_05_HSM)
    target_include_directories(${BINARY_NAME_05_GEN} PRIVATE ${HSMCPP_GLIBMM_INCLUDE} ${CMAKE_BINARY_DIR})
    target_link_libraries(${BINARY_NAME_05_GEN} PRIVATE ${HSMCPP_GLIBMM_LIB})
    target_compile_options(${BINARY_NAME_05_GEN} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()