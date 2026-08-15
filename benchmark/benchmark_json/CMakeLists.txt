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

find_package(jsoncpp)
if(jsoncpp_FOUND)
    target_include_directories(${PROJECT_NAME} PRIVATE jsoncpp_lib)
    target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp_lib)

    target_compile_definitions(
        ${PROJECT_NAME} PRIVATE
        ENABLE_JSONCPP
    )
else()
    message("jsoncpp not found")
endif()
