# ========== 仅头文件库 ==========
function(link_breutil target)
    if(NOT BREUTILS_DIR_PATH)
        message(FATAL_ERROR "BRE_DIR_PATH未设置，请检查平台配置文件")
    endif()

    # 设置预编译头文件（可选）, 目的是加快编译速度
    # target_precompile_headers(${target} PRIVATE
    #     ${BREUTILS_DIR_PATH}/your_needed_header.hpp
    # )

    target_include_directories(${target} PRIVATE ${BREUTILS_DIR_PATH})
endfunction()