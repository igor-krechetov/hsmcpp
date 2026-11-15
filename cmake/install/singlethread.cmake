if (HSMBUILD_DISPATCHER_SINGLETHREAD)
    configure_file(./pkgconfig/hsmcpp_singlethread.pc.in hsmcpp_singlethread.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_singlethread.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_singlethread DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherSingleThread.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cmake/hsmcpp-singlethread.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/)
endif()
