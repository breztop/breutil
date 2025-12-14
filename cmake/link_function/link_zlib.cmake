# zlib
function(link_zlib target)
    # 先使用 CMake 的 ZLIB 包查找，没找到再使用自定义路径
    find_package(ZLIB QUIET)
    if(NOT ZLIB_FOUND)
        if(NOT ZLIB_DIR_PATH)
            message(WARNING "ZLIB_DIR_PATH未设置，请检查平台配置文件")
        endif()

        message(STATUS "ZLIB 未找到，使用自定义路径: ${ZLIB_DIR_PATH}")
        set(ZLIB_INCLUDE_DIRS ${ZLIB_DIR_PATH}/include)
        set(ZLIB_LIBRARIES ${ZLIB_DIR_PATH}/lib/libz.a)
        set(ZLIB_FOUND TRUE)
    endif()
    if(NOT ZLIB_FOUND)
        message(FATAL_ERROR "Zlib 未找到，请检查路径或安装情况")
    endif()
    target_include_directories(${target} PRIVATE ${ZLIB_INCLUDE_DIRS})

    target_link_libraries(${target} PRIVATE ${ZLIB_LIBRARIES})

endfunction(link_zlib target)