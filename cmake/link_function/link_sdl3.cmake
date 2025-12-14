# SDL3 链接函数
function(link_sdl3 target)
    if(NOT SDL3_DIR_PATH)
        message(STATUS "SDL3_DIR_PATH未设置，请检查平台配置文件")
    endif()
    
    set(SDL3_DIR "${SDL3_DIR_PATH}" CACHE PATH "Path to SDL3Config.cmake" FORCE)
    message(STATUS "Setting SDL3_DIR to: ${SDL3_DIR}")
    
    find_package(SDL3 CONFIG 
        HINTS "${SDL3_DIR}"
    )
    
    if(NOT SDL3_FOUND)
        message(WARNING "SDL3未找到，请检查路径: ${SDL3_DIR}")
    else()
        message(STATUS "找到SDL3，版本: ${SDL3_VERSION}")
    endif()
    
    target_link_libraries(${target} PRIVATE SDL3::SDL3)
endfunction()