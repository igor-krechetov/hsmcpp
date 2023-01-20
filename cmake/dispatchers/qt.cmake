if (HSMBUILD_DISPATCHER_QT)
    # For Qt5 build make sure Qt5_DIR environment variable is set. For example:
    # export Qt5_DIR=/home/user/qt/5.13.2/gcc_64/lib/cmake/Qt5/

    # For Qt6 build use qt-cmake

    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    if (WIN32)
        cmake_policy(SET CMP0071 NEW)
        cmake_policy(SET CMP0020 NEW)
    endif()

    if (CMAKE_VERSION VERSION_LESS "3.7.0")
        set(CMAKE_INCLUDE_CURRENT_DIR ON)
    endif()

    # first check for Qt6
    find_package(Qt6 COMPONENTS Core)

    if (NOT Qt6_FOUND)
        if (NOT $ENV{Qt5_DIR} STREQUAL "")
            message("Using Qt5")
            find_package(Qt5 COMPONENTS Core REQUIRED)

            qt5_wrap_cpp(SRC_DISPATCHER_QT ${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/HsmEventDispatcherQt.hpp)
            SET(QtCore_INCLUDE_DIRS ${Qt5Core_INCLUDE_DIRS})
            SET(QtCore_LIBRARIES ${Qt5Core_LIBRARIES})
        else()
            message( FATAL_ERROR "Qt not found")
        endif()
    else()
        message("Using Qt6")
        qt_standard_project_setup()# since Qt 6.3

        qt6_wrap_cpp(SRC_DISPATCHER_QT ${CMAKE_CURRENT_SOURCE_DIR}/include/hsmcpp/HsmEventDispatcherQt.hpp)
        SET(QtCore_INCLUDE_DIRS ${Qt6Core_INCLUDE_DIRS})
        SET(QtCore_LIBRARIES ${Qt6Core_LIBRARIES})
    endif()

    set(HSM_DEFINITIONS_QT ${HSM_DEFINITIONS_BASE} -DDHSM_BUILD_HSMBUILD_DISPATCHER_QT CACHE STRING "" FORCE)
    add_definitions(-DHSM_BUILD_HSMBUILD_DISPATCHER_QT)
    add_library(${HSM_LIBRARY_NAME}_qt STATIC ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmEventDispatcherQt.cpp ${SRC_DISPATCHER_QT})

    if (Qt6_FOUND)
        # Qt6 requires a C++17 compiler
        target_compile_features(${HSM_LIBRARY_NAME}_qt PUBLIC cxx_std_17)
        if (MSVC)
            # Required by Qt
            target_compile_options(${HSM_LIBRARY_NAME}_qt PUBLIC /Zc:__cplusplus /permissive-)
        endif()
    endif()

    target_include_directories(${HSM_LIBRARY_NAME}_qt PUBLIC ${QtCore_INCLUDE_DIRS})

    if (NOT WIN32)
        target_compile_options(${HSM_LIBRARY_NAME}_qt PUBLIC "-fPIC")
    endif()

    set(CMAKE_AUTOMOC OFF)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)

    # Export variables
    set(HSMCPP_QT_CXX_FLAGS ${HSMCPP_CXX_FLAGS} CACHE STRING "" FORCE)
    set(HSMCPP_QT_INCLUDE ${HSMCPP_INCLUDE} ${QtCore_INCLUDE_DIRS} CACHE STRING "" FORCE)
    set(HSMCPP_QT_LIB ${HSM_LIBRARY_NAME}_qt ${HSMCPP_LIB} ${QtCore_LIBRARIES} CACHE STRING "" FORCE)

    registerQtWindeploy()
endif()