# FFmpeg 链接函数
function(link_ffmpeg target)
    # 确保FFMPEG_DIR_PATH已由平台文件设置
    if(NOT FFMPEG_DIR_PATH)
        message(FATAL_ERROR "FFMPEG_DIR_PATH未设置，请检查平台配置文件")
    endif()


    target_include_directories(${target} PRIVATE ${FFMPEG_DIR_PATH}/include)
    target_link_directories(${target} PRIVATE ${FFMPEG_DIR_PATH}/lib)

    # target_include_directories(${target} PRIVATE
    #     -isystem ${FFMPEG_DIR_PATH}/include
    # )

    set(FFMPEG_LIBS avdevice avcodec avfilter avutil swscale avformat swresample postproc)
    foreach(lib ${FFMPEG_LIBS})
        find_library(${lib}_LIB ${lib} PATHS ${FFMPEG_DIR_PATH}/lib NO_DEFAULT_PATH)
        if(NOT ${lib}_LIB)
            message(FATAL_ERROR "找不到FFmpeg库: ${lib}")
        endif()
        list(APPEND FFMPEG_LIBRARIES ${${lib}_LIB})
    endforeach()
    target_link_libraries(${target} PRIVATE ${FFMPEG_LIBRARIES})
endfunction()