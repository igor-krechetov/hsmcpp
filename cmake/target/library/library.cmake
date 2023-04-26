message("Deploy Target: Library")

set(CMAKE_INSTALL_INCLUDEDIR "include/hsmcpp")
set(CMAKE_INSTALL_LIBDIR "lib")

set (DEPLOY_DIR ${CMAKE_INSTALL_INCLUDEDIR})
set (DEPLOY_FILES ${LIBRARY_HEADERS}
                  ${FILES_SCXML2GEN})

foreach(ITEM ${LIBRARY_HEADERS})
    get_filename_component(ITEM_PATH ${ITEM} DIRECTORY)
    get_filename_component(ITEM_NAME ${ITEM} NAME)
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" ITEM_PATH_RELATIVE ${ITEM_PATH})
    if (ITEM_PATH_RELATIVE)
        string(REPLACE "/build" "/" ITEM_PATH_RELATIVE ${ITEM_PATH_RELATIVE})
        string(REPLACE ${CMAKE_INSTALL_INCLUDEDIR} "/" ITEM_PATH_RELATIVE ${ITEM_PATH_RELATIVE})
    endif()

    install(FILES ${ITEM} DESTINATION ${DEPLOY_DIR}/${ITEM_PATH_RELATIVE}/)
endforeach()


# Generate cmake version file
include(CMakePackageConfigHelpers)

write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/hsmcpp-configVersion.cmake
                                 VERSION ${PROJECT_VERSION}
                                 COMPATIBILITY SameMajorVersion )
configure_file(./pkgconfig/hsmcpp.pc.in hsmcpp.pc @ONLY)
install(FILES "${PROJECT_BINARY_DIR}/hsmcpp.pc" DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
install(TARGETS ${HSM_LIBRARY_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pkgconfig/cmake/hsmcpp-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/hsmcpp-configVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/ )
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/tools/scxml2gen/CMakeLists.txt
                ${FILES_SCXML2GEN}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${HSM_LIBRARY_NAME}/scxml2gen)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/install.cmake)


