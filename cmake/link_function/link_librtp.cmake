function(link_librtp target)
    if(NOT LIBRTP_DIR_PATH)
        message(WARNING "LIBRTP_DIR_PATH未设置，请检查平台配置文件")
    endif()

    # 添加详细的调试信息
    message(STATUS "为 ${target} 配置 librtp 库")

    target_include_directories(${target} PRIVATE ${LIBRTP_DIR_PATH}/include)

    if(WIN32)
        target_link_libraries(${target} PRIVATE ${LIBRTP_DIR_PATH}/lib/librtp.lib)
    else()
        target_link_libraries(${target} PRIVATE ${LIBRTP_DIR_PATH}/lib/librtp.a)
    endif()
endfunction()