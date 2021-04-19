# ======================================================
if (HSMBUILD_DISPATCHER_STD)
    set(BINARY_03_NAME "03_debugging")

    # create folder for generated files
    set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
    file(MAKE_DIRECTORY ${GEN_DIR})

    generateHsm(GEN_03_HSM ./03_debugging.scxml "DebugTestHsm" ${GEN_DIR} "GEN_03_OUT_SRC")

    add_executable(${BINARY_03_NAME} 03_debugging.cpp ${GEN_03_OUT_SRC})
    add_dependencies(${BINARY_03_NAME} GEN_03_HSM)
    target_include_directories(${BINARY_03_NAME}
        PRIVATE
            ${HSMCPP_STD_INCLUDE}
            ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${BINARY_03_NAME} PRIVATE ${HSMCPP_STD_LIB})
endif()