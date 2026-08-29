project(benchmark_json)

file(GLOB BENCHMARK_SRC "*.cpp" "*.h")

add_executable(${PROJECT_NAME} ${BENCHMARK_SRC})

# -o2 optimization
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RELEASE>:NDEBUG>
)
target_compile_options(
    ${PROJECT_NAME} PRIVATE
    $<$<CONFIG:RELEASE>:-O3>
)

include_directories(${CMAKE_SOURCE_DIR})
link_benchmark(${PROJECT_NAME})
link_boost(${PROJECT_NAME} json)

find_package(jsoncpp QUIET)
if(jsoncpp_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp_lib)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_JSONCPP)
else()
    message(STATUS "jsoncpp not found; its benchmarks are disabled")
endif()

find_package(simdjson QUIET)
if(simdjson_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE simdjson::simdjson)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_SIMDJSON)
    # simdjson 3.6 triggers GCC 15+'s new template-body diagnostic.
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<AND:$<CXX_COMPILER_ID:GNU>,$<VERSION_GREATER_EQUAL:$<CXX_COMPILER_VERSION>,15>>:-Wno-template-body>
    )
else()
    message(STATUS "simdjson not found; its benchmarks are disabled")
endif()
