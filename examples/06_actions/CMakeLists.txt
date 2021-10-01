if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_06 "06_actions")

    set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
    file(MAKE_DIRECTORY ${GEN_DIR})

    generateHsm(GEN_06_HSM ./06_actions.scxml "ActionsHsm" ${GEN_DIR} "GEN_OUT_SRC_ACTIONS")
    add_executable(${BINARY_NAME_06} 06_actions.cpp ${GEN_OUT_SRC_ACTIONS})
    add_dependencies(${BINARY_NAME_06} GEN_06_HSM)
    target_include_directories(${BINARY_NAME_06} PRIVATE ${HSMCPP_GLIBMM_INCLUDE} ${CMAKE_BINARY_DIR})
    target_link_libraries(${BINARY_NAME_06} PRIVATE ${HSMCPP_GLIBMM_LIB})
    target_compile_options(${BINARY_NAME_06} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()