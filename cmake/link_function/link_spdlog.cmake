function(link_spdlog target)
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

            FetchContent_MakeAvailable(spdlog)

            message(STATUS "spdlog 已下载并添加到项目中")
        endif()
    endif()

    target_link_libraries(${target} PUBLIC spdlog::spdlog)

    target_precompile_headers(${target} PRIVATE <spdlog/spdlog.h>)
endfunction()