function(link_benchmark target)
    find_package(benchmark QUIET)
    if(NOT benchmark_FOUND)
        message(WARNING "benchmark 未找到！请检查路径或安装情况")
        return()
    endif()

    target_link_libraries(${target} PRIVATE benchmark::benchmark)
endfunction()