## FFmpeg 链接函数
## @param target 需要链接FFmpeg库的目标
## @param ... 需要链接的额外FFmpeg库，空则链接默认库

set(FFMPEG_DEFAULT_LIBS
    avdevice
    avcodec
    avfilter
    avutil
    swscale
    avformat
    swresample
)

function(link_ffmpeg target)
    if(ARGC EQUAL 1)
        set(ffmpeg_libs ${FFMPEG_DEFAULT_LIBS})
    else()
        set(ffmpeg_libs ${ARGN})
    endif()

    _link_ffmpeg_normalize_libraries(ffmpeg_libs ${ffmpeg_libs})
    _link_ffmpeg_initialize()

    message(STATUS "${target} 链接 FFmpeg 库: ${ffmpeg_libs}")
    target_link_libraries(${target} PRIVATE ffmpeg::base ${ffmpeg_libs})
endfunction()


function(_link_ffmpeg_normalize_libraries out_var)
    set(normalized_libs)
    foreach(lib IN LISTS ARGN)
        get_filename_component(clean_lib "${lib}" NAME)
        string(REGEX REPLACE "^lib" "" clean_lib "${clean_lib}")
        string(REGEX REPLACE "\\.(a|lib|dll|dylib)$" "" clean_lib "${clean_lib}")
        string(REGEX REPLACE "\\.so(\\.[0-9]+)*$" "" clean_lib "${clean_lib}")
        list(APPEND normalized_libs "${clean_lib}")
    endforeach()

    list(REMOVE_DUPLICATES normalized_libs)
    set(${out_var} "${normalized_libs}" PARENT_SCOPE)
endfunction()

function(_link_ffmpeg_initialize)
    if(TARGET ffmpeg_link_base)
        return()
    endif()

    set(ffmpeg_search_roots
        /usr
        /usr/local
        /opt/local
        /opt/homebrew
        /usr/local/opt/ffmpeg
    )

    foreach(env_name FFMPEG_ROOT FFMPEG_DIR FFMPEG_HOME)
        if(DEFINED ENV{${env_name}} AND NOT "$ENV{${env_name}}" STREQUAL "")
            list(APPEND ffmpeg_search_roots "$ENV{${env_name}}")
        endif()
    endforeach()

    if(DEFINED FFMPEG_DIR_PATH AND NOT "${FFMPEG_DIR_PATH}" STREQUAL "")
        list(APPEND ffmpeg_search_roots "${FFMPEG_DIR_PATH}")
    endif()

    list(REMOVE_DUPLICATES ffmpeg_search_roots)

    set(ffmpeg_include_dirs)
    set(ffmpeg_library_dirs)
    set(ffmpeg_compile_options)
    set(ffmpeg_link_options)
    set(ffmpeg_found FALSE)
    set(ffmpeg_source "")

    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(PC_LIBAVCODEC QUIET libavcodec)
        if(PC_LIBAVCODEC_FOUND)
            set(ffmpeg_found TRUE)
            set(ffmpeg_source "pkg-config")
            list(APPEND ffmpeg_include_dirs ${PC_LIBAVCODEC_INCLUDE_DIRS})
            list(APPEND ffmpeg_library_dirs ${PC_LIBAVCODEC_LIBRARY_DIRS})
            list(APPEND ffmpeg_compile_options ${PC_LIBAVCODEC_CFLAGS_OTHER})
            list(APPEND ffmpeg_link_options ${PC_LIBAVCODEC_LDFLAGS_OTHER})
        endif()
    endif()

    if(NOT ffmpeg_found)
        find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h
            HINTS ${ffmpeg_search_roots}
            PATH_SUFFIXES include
        )

        find_library(FFMPEG_AVCODEC_LIB avcodec
            HINTS ${ffmpeg_search_roots}
            PATH_SUFFIXES lib lib64 bin
        )

        if(FFMPEG_INCLUDE_DIR AND FFMPEG_AVCODEC_LIB)
            set(ffmpeg_found TRUE)
            set(ffmpeg_source "path search")
            get_filename_component(FFMPEG_LIB_DIR "${FFMPEG_AVCODEC_LIB}" DIRECTORY)
            list(APPEND ffmpeg_include_dirs "${FFMPEG_INCLUDE_DIR}")
            list(APPEND ffmpeg_library_dirs "${FFMPEG_LIB_DIR}")
        endif()
    endif()

    if(NOT ffmpeg_found)
        if(DEFINED FFMPEG_DIR_PATH AND NOT "${FFMPEG_DIR_PATH}" STREQUAL "")
            if(NOT EXISTS "${FFMPEG_DIR_PATH}")
                message(FATAL_ERROR "FFMPEG_DIR_PATH不存在: ${FFMPEG_DIR_PATH}")
            endif()

            message(FATAL_ERROR "未能从 pkg-config 或 FFMPEG_DIR_PATH 找到 FFmpeg: ${FFMPEG_DIR_PATH}")
        endif()

        message(FATAL_ERROR "未找到FFmpeg。请安装 pkg-config 和 FFmpeg 开发包，或设置 FFMPEG_ROOT/FFMPEG_DIR/FFMPEG_HOME/FFMPEG_DIR_PATH")
    endif()

    list(REMOVE_DUPLICATES ffmpeg_include_dirs)
    list(REMOVE_DUPLICATES ffmpeg_library_dirs)
    list(REMOVE_DUPLICATES ffmpeg_compile_options)
    list(REMOVE_DUPLICATES ffmpeg_link_options)

    add_library(ffmpeg_link_base INTERFACE)
    add_library(ffmpeg::base ALIAS ffmpeg_link_base)

    if(ffmpeg_include_dirs)
        target_include_directories(ffmpeg_link_base INTERFACE ${ffmpeg_include_dirs})
    endif()

    if(ffmpeg_library_dirs)
        target_link_directories(ffmpeg_link_base INTERFACE ${ffmpeg_library_dirs})
    endif()

    if(ffmpeg_compile_options)
        target_compile_options(ffmpeg_link_base INTERFACE ${ffmpeg_compile_options})
    endif()

    if(ffmpeg_link_options)
        target_link_options(ffmpeg_link_base INTERFACE ${ffmpeg_link_options})
    endif()

    message(STATUS "FFmpeg 初始化完成，来源: ${ffmpeg_source}")
    message(STATUS "FFmpeg include dirs: ${ffmpeg_include_dirs}")
    message(STATUS "FFmpeg library dirs: ${ffmpeg_library_dirs}")
endfunction()

