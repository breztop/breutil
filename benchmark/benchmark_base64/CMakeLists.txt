project(benchmark_base64)

function(add_base64_benchmark mode use_simd)
    set(target benchmark_base64_${mode})
    add_executable(${target} benchmark_base64.cpp)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_definitions(${target} PRIVATE BRE_BASE64_USE_SIMD=${use_simd})
    target_compile_options(${target} PRIVATE
        $<$<CONFIG:Release>:-O2>
    )
    link_benchmark(${target})
endfunction()

include(CheckIncludeFileCXX)
check_include_file_cxx("experimental/simd" BREUTIL_HAS_EXPERIMENTAL_SIMD)

add_base64_benchmark(scalar 0)
if(BREUTIL_HAS_EXPERIMENTAL_SIMD)
    add_base64_benchmark(simd 1)
else()
    message(STATUS "<experimental/simd> not found; SIMD Base64 benchmark is disabled")
endif()
