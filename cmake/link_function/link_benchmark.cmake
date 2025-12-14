function(link_benchmark target)
    find_package(benchmark)
    if(NOT benchmark_FOUND)
        message(WARNING "benchmark 未找到！请检查路径或安装情况")
    endif()

    target_include_directories(${target} PRIVATE benchmark::benchmark)
    target_link_libraries(${target} PRIVATE ${benchmark_LIBRARIES} benchmark::benchmark)
endfunction()