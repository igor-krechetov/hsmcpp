if (HSMBUILD_DISPATCHER_SINGLETHREAD)
    set(HSM_DEFINITIONS_SINGLETHREAD ${HSM_DEFINITIONS_BASE} -DHSM_BUILD_HSMBUILD_DISPATCHER_SINGLETHREAD CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_SINGLETHREAD)
    add_library(${HSM_LIBRARY_NAME}_singlethread STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherSingleThread.cpp)

    if (NOT WIN32)
        target_compile_options(${HSM_LIBRARY_NAME}_singlethread PUBLIC "-fPIC")
    endif()

    # Export variables
    # NOTE: no -pthread here. This dispatcher owns no worker thread; it is driven by the
    #       application's own loop. It is the dispatcher intended to pair with HSMBUILD_THREAD_SAFETY=OFF.
    set(HSMCPP_SINGLETHREAD_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_SINGLETHREAD_LIB ${HSM_LIBRARY_NAME}_singlethread ${HSMCPP_LIB} CACHE STRING "" FORCE)
    set(HSMCPP_SINGLETHREAD_INCLUDE ${HSMCPP_INCLUDE} CACHE STRING "" FORCE)
endif()
