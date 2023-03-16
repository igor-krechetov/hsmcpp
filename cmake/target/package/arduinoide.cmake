message("Deploy Target: Arduino IDE")

set (DEPLOY_DIR ${DEPLOY_DIR_ROOT}/arduinoide)
set (EXAMPLES_DIR ${DEPLOY_DIR}/examples)
set (DEPLOY_FILES ${LIBRARY_SRC}
                  ${LIBRARY_HEADERS}
                  ${CMAKE_CURRENT_SOURCE_DIR}/src/HsmImpl.hpp
                  ${FILES_SCXML2GEN}
                  ${CMAKE_CURRENT_SOURCE_DIR}/README.md
                  ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md
                  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE)

foreach(ITEM ${DEPLOY_FILES})
    get_filename_component(ITEM_PATH ${ITEM} DIRECTORY)
    get_filename_component(ITEM_NAME ${ITEM} NAME)
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" ITEM_PATH_RELATIVE ${ITEM_PATH})
    if (ITEM_PATH_RELATIVE)
        string(REPLACE "/include/" "/src/" ITEM_PATH_RELATIVE ${ITEM_PATH_RELATIVE})
    endif()

    install(FILES ${ITEM} DESTINATION ${DEPLOY_DIR}/${ITEM_PATH_RELATIVE}/)
endforeach()

# Install root header
install(FILES ${CMAKE_CURRENT_LIST_DIR}/hsmcpp.hpp DESTINATION ${DEPLOY_DIR}/src)

# Install examples
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/09_arduino/01_blink/src/main.cpp DESTINATION ${EXAMPLES_DIR}/01_blink RENAME 01_blink.ino)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/09_arduino/01_blink/blink.scxml DESTINATION ${EXAMPLES_DIR}/01_blink)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/09_arduino/02_blink_button/src/main.cpp DESTINATION ${EXAMPLES_DIR}/02_blink_button RENAME 02_blink_button.ino)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/examples/09_arduino/02_blink_button/blink_button.scxml DESTINATION ${EXAMPLES_DIR}/02_blink_button)

# Arduino IDE library manifest
configure_file(${CMAKE_CURRENT_LIST_DIR}/arduinoide/library.properties.in ${DEPLOY_DIR}/library.properties @ONLY)
