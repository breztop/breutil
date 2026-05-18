function(copy_directory SOURCE_DIR DEST_DIR)
    if(NOT EXISTS "${SOURCE_DIR}")
        message(FATAL_ERROR "Source directory '${SOURCE_DIR}' does not exist.")
    endif()

    file(COPY "${SOURCE_DIR}/" DESTINATION "${DEST_DIR}")
    message(STATUS "Copied directory '${SOURCE_DIR}' to '${DEST_DIR}'")
endfunction()
