# ======================================================
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_00_1 "00_helloworld_glibmm")

    add_executable(${BINARY_NAME_00_1} 00_helloworld_glibmm.cpp)
    target_include_directories(${BINARY_NAME_00_1} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${BINARY_NAME_00_1} PRIVATE ${HSMCPP_GLIBMM_LIB})
    target_compile_options(${BINARY_NAME_00_1} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()

# ======================================================
if (HSMBUILD_DISPATCHER_STD)
    set(BINARY_NAME_00_2 "00_helloworld_std")

    add_executable(${BINARY_NAME_00_2} 00_helloworld_std.cpp)
    target_include_directories(${BINARY_NAME_00_2} PRIVATE ${HSMCPP_STD_INCLUDE})
    target_link_libraries(${BINARY_NAME_00_2} PRIVATE ${HSMCPP_STD_LIB})
    target_compile_options(${BINARY_NAME_00_2} PRIVATE ${HSMCPP_STD_CXX_FLAGS})
endif()

# ======================================================
if (HSMBUILD_DISPATCHER_QT)
    set(BINARY_NAME_00_3 "00_helloworld_qt")

    add_executable(${BINARY_NAME_00_3} 00_helloworld_qt.cpp)
    target_include_directories(${BINARY_NAME_00_3} PRIVATE ${HSMCPP_QT_INCLUDE})
    target_link_libraries(${BINARY_NAME_00_3} PRIVATE ${HSMCPP_QT_LIB})
    target_compile_options(${BINARY_NAME_00_3} PRIVATE ${HSMCPP_QT_CXX_FLAGS})
    deployQtApp(${BINARY_NAME_00_3})
endif()
