function(link_spdlog target)
    set(scope PRIVATE)
    set(spdlog_target spdlog::spdlog)

    foreach(arg IN LISTS ARGN)
        if(arg MATCHES "^(PRIVATE|PUBLIC|INTERFACE)$")
            set(scope ${arg})
        elseif(arg STREQUAL "HEADER_ONLY")
            set(spdlog_target spdlog::spdlog_header_only)
        else()
            message(FATAL_ERROR "link_spdlog(${target}): unknown argument '${arg}'")
        endif()
    endforeach()

    if(NOT TARGET spdlog::spdlog)
        find_package(spdlog QUIET)
        if(NOT spdlog_FOUND)
            include(FetchContent)

            FetchContent_Declare(
                spdlog
                GIT_REPOSITORY https://github.com/gabime/spdlog.git
                GIT_TAG        v1.17.0  # 请根据需要调整版本
            )
            set(SPDLOG_USE_STD_FORMAT OFF CACHE BOOL "Use std::format for spdlog" FORCE)

            if(WIN32)
                set(SPDLOG_BUILD_SHARED ON CACHE BOOL "Build spdlog as a shared library" FORCE)
            else()
                set(SPDLOG_BUILD_SHARED OFF CACHE BOOL "Build spdlog as a shared library" FORCE)
            endif()
            set(SPDLOG_BUILD_PIC ON CACHE BOOL "" FORCE)
            set(SPDLOG_INSTALL ON CACHE BOOL "Install BreFlow's spdlog dependency" FORCE)

            FetchContent_MakeAvailable(spdlog)

            message(STATUS "spdlog 已下载并添加到项目中")
        else()
            message(STATUS "Found spdlog at: ${spdlog_DIR}, version: ${spdlog_VERSION}")
        endif()
    endif()

    if(NOT TARGET ${spdlog_target})
        message(FATAL_ERROR "The requested spdlog target '${spdlog_target}' is unavailable")
    endif()

    target_link_libraries(${target} ${scope} ${spdlog_target})
    message(STATUS "${target} link ${spdlog_target}")

    target_precompile_headers(${target} ${scope} <spdlog/spdlog.h>)
endfunction()
