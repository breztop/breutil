## boost 链接函数，支持自动查找和链接指定组件
# @param target 要链接 Boost 的目标
# @param ... 可选的 Boost 组件列表（如 system, filesystem 等）
# 如果未指定组件，则仅链接头文件（适用于 header-only 库）
function(link_boost target)
    set(components ${ARGN})
    list(REMOVE_DUPLICATES components)

    _link_boost_initialize()
    _link_boost_find_missing_components(${components})

    foreach(comp ${components})
        if(TARGET Boost::${comp})
            target_link_libraries(${target} PRIVATE Boost::${comp})
        else()
            message(FATAL_ERROR "Boost 组件 ${comp} 的目标不存在，请检查组件名称是否正确")
        endif()
    endforeach()

    # 同时链接头文件目标（确保包含路径正确）
    target_link_libraries(${target} PRIVATE Boost::headers)
    if (components)
        message(STATUS "target ${target} 已链接 Boost 组件: ${components}")
    else()
        message(STATUS "target ${target} 已链接 Boost 头文件")
    endif()
endfunction()

function(_link_boost_collect_search_roots out_var)
    set(boost_search_roots
        /usr
        /usr/local
        /opt/local
        /opt/homebrew
        /usr/local/opt/boost
    )

    foreach(env_name BOOST_ROOT BOOST_DIR BOOST_HOME)
        if(DEFINED ENV{${env_name}} AND NOT "$ENV{${env_name}}" STREQUAL "")
            list(APPEND boost_search_roots "$ENV{${env_name}}")
        endif()
    endforeach()

    if(DEFINED BOOST_DIR_PATH AND NOT "${BOOST_DIR_PATH}" STREQUAL "")
        list(APPEND boost_search_roots "${BOOST_DIR_PATH}")
    endif()

    list(REMOVE_DUPLICATES boost_search_roots)
    set(${out_var} "${boost_search_roots}" PARENT_SCOPE)
endfunction()

function(_link_boost_print_basic_info)
    if(DEFINED _BOOST_INFO_PRINTED)
        return()
    endif()

    message(STATUS "Boost found: ${Boost_FOUND}")
    message(STATUS "Boost include dirs: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost version: ${Boost_VERSION}")
    set(_BOOST_INFO_PRINTED TRUE CACHE INTERNAL "Boost basic info has been printed")
endfunction()

function(_link_boost_ensure_headers_target)
    if(TARGET Boost::headers)
        return()
    endif()

    add_library(Boost::headers INTERFACE IMPORTED)
    set_target_properties(Boost::headers PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${Boost_INCLUDE_DIRS}"
    )

    if(DEFINED Boost_DEFINITIONS)
        set_target_properties(Boost::headers PROPERTIES
            INTERFACE_COMPILE_DEFINITIONS "${Boost_DEFINITIONS}"
        )
    endif()
endfunction()

function(_link_boost_collect_missing_components out_var)
    set(missing_components)
    foreach(comp IN LISTS ARGN)
        if(NOT TARGET Boost::${comp})
            list(APPEND missing_components ${comp})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES missing_components)
    set(${out_var} "${missing_components}" PARENT_SCOPE)
endfunction()

function(_link_boost_initialize)
    if(TARGET Boost::headers)
        return()
    endif()

    _link_boost_collect_search_roots(boost_search_roots)

    find_package(Boost REQUIRED
        HINTS ${boost_search_roots}
    )

    if(NOT Boost_FOUND)
        message(FATAL_ERROR "can't find boost! please set BOOST_ROOT/BOOST_DIR/BOOST_HOME/BOOST_DIR_PATH or install boost")
    endif()

    _link_boost_print_basic_info()
    _link_boost_ensure_headers_target()
    set(_BOOST_INITIALIZED TRUE CACHE INTERNAL "Boost has been initialized")
endfunction()

function(_link_boost_find_missing_components)
    if(NOT ARGN)
        return()
    endif()

    _link_boost_collect_missing_components(missing_components ${ARGN})
    if(NOT missing_components)
        return()
    endif()

    _link_boost_collect_search_roots(boost_search_roots)

    find_package(Boost REQUIRED
        COMPONENTS ${missing_components}
        HINTS ${boost_search_roots}
    )

    if(NOT Boost_FOUND)
        message(FATAL_ERROR "can't find boost! please set BOOST_ROOT/BOOST_DIR/BOOST_HOME/BOOST_DIR_PATH or install boost")
    endif()
endfunction()
