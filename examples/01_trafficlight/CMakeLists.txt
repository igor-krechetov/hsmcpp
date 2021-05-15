# ======================================================
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_01_1 "01_trafficlight")

    add_executable(${BINARY_NAME_01_1} 01_trafficlight.cpp)
    target_include_directories(${BINARY_NAME_01_1} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${BINARY_NAME_01_1} PRIVATE ${HSMCPP_GLIBMM_LIB} ${CMAKE_THREAD_LIBS_INIT})
    target_compile_options(${BINARY_NAME_01_1} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()

# ======================================================
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_01_2 "01_trafficlight_class")

    add_executable(${BINARY_NAME_01_2} 01_trafficlight_class.cpp)
    target_include_directories(${BINARY_NAME_01_2} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${BINARY_NAME_01_2} PRIVATE ${HSMCPP_GLIBMM_LIB} ${CMAKE_THREAD_LIBS_INIT})
    target_compile_options(${BINARY_NAME_01_2} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()

# ======================================================
if (HSMBUILD_DISPATCHER_GLIBMM)
    set(BINARY_NAME_01_3 "01_trafficlight_lambda")

    add_executable(${BINARY_NAME_01_3} 01_trafficlight_lambda.cpp)
    target_include_directories(${BINARY_NAME_01_3} PRIVATE ${HSMCPP_GLIBMM_INCLUDE})
    target_link_libraries(${BINARY_NAME_01_3} PRIVATE ${HSMCPP_GLIBMM_LIB} ${CMAKE_THREAD_LIBS_INIT})
    target_compile_options(${BINARY_NAME_01_3} PRIVATE ${HSMCPP_GLIBMM_CXX_FLAGS})
endif()
