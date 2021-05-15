set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HSMCPP_CXX_FLAGS}")

# ======================================================
if (HSMBUILD_DISPATCHER_STD)
    set(BINARY_NAME_02 "02_generated")

    # create folder for generated files
    set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
    file(MAKE_DIRECTORY ${GEN_DIR})

    generateHsm(GEN_02_HSM ./02_generated.scxml "SwitchHsm" ${GEN_DIR} "GEN_OUT_SRC")

    add_executable(${BINARY_NAME_02} 02_generated.cpp ${GEN_OUT_SRC})
    add_dependencies(${BINARY_NAME_02} GEN_02_HSM)
    target_include_directories(${BINARY_NAME_02}
        PRIVATE
            ${HSMCPP_STD_INCLUDE}
            ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${BINARY_NAME_02} PRIVATE ${HSMCPP_STD_LIB})
    target_compile_options(${BINARY_NAME_02} PRIVATE ${HSMCPP_STD_CXX_FLAGS})
endif()