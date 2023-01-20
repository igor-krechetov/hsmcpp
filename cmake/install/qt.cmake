if (HSMBUILD_DISPATCHER_QT)
    configure_file(./pkgconfig/hsmcpp_qt.pc.in hsmcpp_qt.pc @ONLY)
    install(FILES "${PROJECT_BINARY_DIR}/hsmcpp_qt.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(TARGETS ${HSM_LIBRARY_NAME}_qt DESTINATION ${CMAKE_INSTALL_LIBDIR})
    install(FILES ${HSM_INCLUDES_ROOT}/HsmEventDispatcherQt.hpp
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()