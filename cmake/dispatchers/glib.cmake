if (HSMBUILD_DISPATCHER_GLIB)
    pkg_check_modules(GLIB REQUIRED glib-2.0)

    set(HSM_DEFINITIONS_GLIB ${HSM_DEFINITIONS_BASE} "-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB" CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_GLIB)
    add_library(${HSM_LIBRARY_NAME}_glib STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherGLib.cpp)
    target_include_directories(${HSM_LIBRARY_NAME}_glib PUBLIC ${GLIB_INCLUDE_DIRS})
    target_compile_options(${HSM_LIBRARY_NAME}_glib PUBLIC "-fPIC")

    # Export variables
    set(HSMCPP_GLIB_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIB_LIB ${HSM_LIBRARY_NAME}_glib ${HSMCPP_LIB} ${GLIB_LDFLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_GLIB_INCLUDE ${HSMCPP_INCLUDE} ${GLIB_INCLUDE_DIRS} CACHE STRING "" FORCE)
endif()