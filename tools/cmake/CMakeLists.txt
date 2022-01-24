function(registerQtWindeploy)
    if(Qt5_FOUND AND WIN32)
        get_target_property(_qt5_qmake_location Qt5::qmake IMPORTED_LOCATION)

        execute_process(
            COMMAND "${_qt5_qmake_location}" -query QT_INSTALL_PREFIX
            RESULT_VARIABLE return_code
            OUTPUT_VARIABLE qt5_install_prefix
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        set(imported_location "${qt5_install_prefix}/bin/windeployqt.exe")
        message("imported_location=${imported_location}")

        if(EXISTS ${imported_location})
            add_executable(Qt5::windeployqt IMPORTED)
            set_target_properties(Qt5::windeployqt PROPERTIES
                                  IMPORTED_LOCATION ${imported_location})
        endif()
    endif()
endfunction()

function(deployQtApp _target)
    if(Qt5_FOUND AND WIN32)
        add_custom_command(TARGET ${_target}
            POST_BUILD
            # COMMAND Qt5::windeployqt --dir "${CMAKE_CURRENT_BINARY_DIR}/windeployqt" "$<TARGET_FILE_DIR:foo>/$<TARGET_FILE_NAME:foo>"
            COMMAND Qt5::windeployqt $<TARGET_FILE:${_target}>
        )
    endif()
endfunction()

function(installGoogleTest)
    include(FetchContent)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(googletest
                         GIT_REPOSITORY https://github.com/google/googletest.git
                         GIT_TAG 703bd9caab50b139428cea1aaff9974ebee5742e # release-1.10.0
    )
    if (WIN32)
        # Prevent overriding the parent project's compiler/linker settings
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    endif()
    FetchContent_MakeAvailable(googletest)
endfunction()