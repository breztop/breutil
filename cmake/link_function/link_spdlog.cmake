# spdlog 链接函数
set(SPDLOG_DIR_PATH "${CMAKE_SOURCE_DIR}/thirdLib/spdlog")
function(link_spdlog target)
    # 设置 spdlog 的包含目录
    include_directories(${SPDLOG_DIR_PATH}/include)
endfunction()