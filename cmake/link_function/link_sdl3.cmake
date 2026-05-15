# SDL3 链接函数
# @param target 需要链接 SDL3 库的目标
function(link_sdl3 target)
    
    message(STATUS "${target} 正在链接 SDL3")
    
    _link_sdl3_initialize()
    
    if(TARGET SDL3::SDL3)
        target_link_libraries(${target} PRIVATE SDL3::SDL3)
    else()
        message(FATAL_ERROR "SDL3 目标未找到，请检查 SDL3 安装")
    endif()
endfunction()

function(_link_sdl3_initialize)
    if(TARGET SDL3::SDL3)
        return()
    endif()

    set(sdl3_search_roots
        /usr
        /usr/local
        /opt/local
        /opt/homebrew
        /usr/local/opt/sdl3
    )

    # 检查环境变量
    foreach(env_name SDL3_ROOT SDL3_DIR SDL3_HOME)
        if(DEFINED ENV{${env_name}} AND NOT "$ENV{${env_name}}" STREQUAL "")
            list(APPEND sdl3_search_roots "$ENV{${env_name}}")
        endif()
    endforeach()

    if(DEFINED SDL3_DIR_PATH AND NOT "${SDL3_DIR_PATH}" STREQUAL "")
        list(APPEND sdl3_search_roots "${SDL3_DIR_PATH}")
    endif()

    list(REMOVE_DUPLICATES sdl3_search_roots)

    set(sdl3_include_dirs)
    set(sdl3_library_dirs)
    set(sdl3_compile_options)
    set(sdl3_link_options)
    set(sdl3_found FALSE)
    set(sdl3_source "")
    set(sdl3_version "")

    if(NOT sdl3_found)
        find_package(SDL3 QUIET CONFIG
            HINTS ${sdl3_search_roots}
        )
        
        if(SDL3_FOUND AND TARGET SDL3::SDL3)
            set(sdl3_found TRUE)
            set(sdl3_source "SDL3Config.cmake")
            get_target_property(sdl3_include_dirs SDL3::SDL3 INTERFACE_INCLUDE_DIRECTORIES)
            get_target_property(sdl3_compile_options SDL3::SDL3 INTERFACE_COMPILE_OPTIONS)
            get_target_property(sdl3_link_options SDL3::SDL3 INTERFACE_LINK_OPTIONS)
        endif()
    endif()

    # 如果 Config 模式失败，尝试 pkg-config
    if(NOT sdl3_found)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(PC_SDL3 QUIET sdl3)
            if(PC_SDL3_FOUND)
                set(sdl3_found TRUE)
                set(sdl3_source "pkg-config")
                set(sdl3_version "${PC_SDL3_VERSION}")
                list(APPEND sdl3_include_dirs ${PC_SDL3_INCLUDE_DIRS})
                list(APPEND sdl3_library_dirs ${PC_SDL3_LIBRARY_DIRS})
                list(APPEND sdl3_compile_options ${PC_SDL3_CFLAGS_OTHER})
                list(APPEND sdl3_link_options ${PC_SDL3_LDFLAGS_OTHER})
            endif()
        endif()
    endif()

    # 如果 pkg-config 失败，尝试手动查找
    if(NOT sdl3_found)
        find_path(SDL3_INCLUDE_DIR SDL3/SDL.h
            HINTS ${sdl3_search_roots}
            PATH_SUFFIXES include
        )

        find_library(SDL3_LIBRARY
            NAMES SDL3
            HINTS ${sdl3_search_roots}
            PATH_SUFFIXES lib lib64 bin
        )

        if(SDL3_INCLUDE_DIR AND SDL3_LIBRARY)
            set(sdl3_found TRUE)
            set(sdl3_source "manual search")
            get_filename_component(SDL3_LIB_DIR "${SDL3_LIBRARY}" DIRECTORY)
            list(APPEND sdl3_include_dirs "${SDL3_INCLUDE_DIR}")
            list(APPEND sdl3_library_dirs "${SDL3_LIB_DIR}")
        endif()
    endif()

    if(NOT sdl3_found)
        if(DEFINED SDL3_DIR_PATH AND NOT "${SDL3_DIR_PATH}" STREQUAL "")
            if(NOT EXISTS "${SDL3_DIR_PATH}")
                message(FATAL_ERROR "SDL3_DIR_PATH不存在: ${SDL3_DIR_PATH}")
            endif()
            message(FATAL_ERROR "can't find SDL3Config.cmake、pkg-config or SDL3_DIR_PATH path: ${SDL3_DIR_PATH}")
        endif()
        message(FATAL_ERROR "can't find SDL3. set SDL3_ROOT/SDL3_DIR/SDL3_HOME/SDL3_DIR_PATH environment variables or install pkg-config and SDL3 development package")
    endif()

    # 移除重复项
    list(REMOVE_DUPLICATES sdl3_include_dirs)
    list(REMOVE_DUPLICATES sdl3_library_dirs)
    list(REMOVE_DUPLICATES sdl3_compile_options)
    list(REMOVE_DUPLICATES sdl3_link_options)

    # 创建别名目标
    if(NOT TARGET SDL3::SDL3)
        add_library(SDL3::SDL3 UNKNOWN IMPORTED)
        
        if(SDL3_LIBRARY)
            set_target_properties(SDL3::SDL3 PROPERTIES
                IMPORTED_LOCATION "${SDL3_LIBRARY}"
            )
        endif()
        
        if(sdl3_include_dirs)
            set_target_properties(SDL3::SDL3 PROPERTIES
                INTERFACE_INCLUDE_DIRECTORIES "${sdl3_include_dirs}"
            )
        endif()
        
        if(sdl3_library_dirs)
            set_target_properties(SDL3::SDL3 PROPERTIES
                INTERFACE_LINK_DIRECTORIES "${sdl3_library_dirs}"
            )
        endif()
        
        if(sdl3_compile_options)
            set_target_properties(SDL3::SDL3 PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${sdl3_compile_options}"
            )
        endif()
        
        if(sdl3_link_options)
            set_target_properties(SDL3::SDL3 PROPERTIES
                INTERFACE_LINK_OPTIONS "${sdl3_link_options}"
            )
        endif()
    endif()

    message(STATUS "SDL3 initialization complete, source: ${sdl3_source}")
    if(sdl3_version)
        message(STATUS "SDL3 version: ${sdl3_version}")
    endif()
    if(sdl3_include_dirs)
        message(STATUS "SDL3 include dirs: ${sdl3_include_dirs}")
    endif()
    if(sdl3_library_dirs)
        message(STATUS "SDL3 library dirs: ${sdl3_library_dirs}")
    endif()
endfunction()