function(link_nlohmann_json target)
    if(NOT TARGET nlohmann_json::nlohmann_json)
        find_package(nlohmann_json QUIET)
        if(NOT nlohmann_json_FOUND)
            include(FetchContent)

            FetchContent_Declare(
                json
                GIT_REPOSITORY https://github.com/nlohmann/json.git
                GIT_TAG        v3.12.0
            )

            FetchContent_MakeAvailable(json)

            get_target_property(inc_dirs nlohmann_json::nlohmann_json INTERFACE_INCLUDE_DIRECTORIES)
            message(STATUS "nlohmann_json 库的包含路径: ${inc_dirs}")

            message(STATUS "nlohmann_json 已下载并添加到项目中")
        endif()
    endif()
    
    target_link_libraries(${target} PUBLIC nlohmann_json::nlohmann_json)
    target_precompile_headers(${target} PRIVATE <nlohmann/json.hpp>)
endfunction()