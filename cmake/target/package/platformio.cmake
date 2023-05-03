message("Deploy Target: PlatformIO")

set (DEPLOY_DIR ${DEPLOY_DIR_ROOT}/platformio)
set (DEPLOY_FILES ${LIBRARY_SRC}
                  ${LIBRARY_HEADERS}
                  ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmImpl.hpp
                  ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmImplTypes.hpp
                  ${FILES_SCXML2GEN}
                  ${CMAKE_CURRENT_SOURCE_DIR}/README.md
                  ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
                  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

foreach(ITEM ${DEPLOY_FILES})
    get_filename_component(ITEM_PATH ${ITEM} DIRECTORY)
    get_filename_component(ITEM_NAME ${ITEM} NAME)
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" ITEM_PATH_RELATIVE ${ITEM_PATH})
    if (ITEM_PATH_RELATIVE)
        string(REPLACE "/build" "/include/hsmcpp" ITEM_PATH_RELATIVE ${ITEM_PATH_RELATIVE})
    endif()

    install(FILES ${ITEM} DESTINATION ${DEPLOY_DIR}/${ITEM_PATH_RELATIVE}/)
endforeach()

# Install root header
install(FILES ${CMAKE_CURRENT_LIST_DIR}/hsmcpp.hpp DESTINATION ${DEPLOY_DIR}/include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/examples/09_arduino/ DESTINATION ${DEPLOY_DIR}/examples)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/platformio/hsmcpp_pio_integration.py DESTINATION ${DEPLOY_DIR}/)
configure_file(${CMAKE_CURRENT_LIST_DIR}/platformio/library.json.in ${DEPLOY_DIR}/library.json @ONLY)