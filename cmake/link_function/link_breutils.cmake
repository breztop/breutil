# ========== 仅头文件库 ==========
function(link_breutils target)
    if(NOT BREUTILS_DIR_PATH)
        message(FATAL_ERROR "BRE_DIR_PATH未设置，请检查平台配置文件")
    endif()

    # 设置预编译头文件（可选）, 目的是加快编译速度
    # target_precompile_headers(${target} PRIVATE
    #     ${BREUTILS_DIR_PATH}/breUtils/block_queue.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/spdlog.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/time_test.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/platform.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/file_writer.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/defer.hpp
    #     ${BREUTILS_DIR_PATH}/breUtils/hex_look.hpp
    # )

    target_include_directories(${target} PRIVATE ${BREUTILS_DIR_PATH})
endfunction()