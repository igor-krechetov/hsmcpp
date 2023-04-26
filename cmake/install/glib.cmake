if (HSMBUILD_DISPATCHER_GLIB)
    configure_file(./pkgconfig/hsmcpp_glib.pc.in hsmcpp_glib.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_glib.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_glib DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherGLib.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cmake/hsmcpp-glib.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/)
endif()