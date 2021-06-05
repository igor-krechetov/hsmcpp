set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${HSMCPP_CXX_FLAGS}")

# ======================================================
if (HSMBUILD_DISPATCHER_STD)
    set(BINARY_NAME_04 "04_history")

    # create folder for generated files
    set(GEN_DIR ${CMAKE_BINARY_DIR}/gen)
    set(GEN_DIR_DIAGRAM ${CMAKE_BINARY_DIR}/gen_diagram)
    file(MAKE_DIRECTORY ${GEN_DIR})
    file(MAKE_DIRECTORY ${GEN_DIR_DIAGRAM})

    generateHsm(GEN_04_HSM ./04_history.scxml "PlayerHsm" ${GEN_DIR} "GEN_PLAYERHSM_SRC")
    generateHsmDiagram(GEN_04_HSM_DIAGRAM ./04_history.scxml ${GEN_DIR_DIAGRAM}/04_history.plantuml)

    add_executable(${BINARY_NAME_04} 04_history.cpp ${GEN_PLAYERHSM_SRC})
    add_dependencies(${BINARY_NAME_04} GEN_04_HSM)
    add_dependencies(${BINARY_NAME_04} GEN_04_HSM_DIAGRAM)
    target_include_directories(${BINARY_NAME_04}
        PRIVATE
            ${HSMCPP_STD_INCLUDE}
            ${CMAKE_BINARY_DIR}
    )
    target_link_libraries(${BINARY_NAME_04} PRIVATE ${HSMCPP_STD_LIB})
    target_compile_options(${BINARY_NAME_04} PRIVATE ${HSMCPP_STD_CXX_FLAGS})
endif()