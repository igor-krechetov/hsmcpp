if (HSMBUILD_DISPATCHER_GLIBMM)
    configure_file(./pkgconfig/hsmcpp_glibmm.pc.in hsmcpp_glibmm.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_glibmm.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_glibmm DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherGLibmm.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cmake/hsmcpp-glibmm.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/)
endif()