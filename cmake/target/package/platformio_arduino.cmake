message("Deploy Target: PlatformIO-Arduino")

set (DEPLOY_FILES ${LIBRARY_SRC} ${LIBRARY_HEADERS} ${FILES_SCXML2GEN})

foreach(ITEM ${DEPLOY_FILES})
    get_filename_component(ITEM_PATH ${ITEM} DIRECTORY)
    get_filename_component(ITEM_NAME ${ITEM} NAME)
    string(REPLACE ${CMAKE_CURRENT_SOURCE_DIR} "" ITEM_PATH_RELATIVE ${ITEM_PATH})

    install(FILES ${ITEM} DESTINATION ${DEPLOY_DIR}/${ITEM_PATH_RELATIVE}/)
endforeach()

configure_file(./platformio/library.json.in ${DEPLOY_DIR}/library.json @ONLY)