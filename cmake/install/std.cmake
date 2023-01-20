if (HSMBUILD_DISPATCHER_STD)
    configure_file(./pkgconfig/hsmcpp_std.pc.in hsmcpp_std.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_std.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_std DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherSTD.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()