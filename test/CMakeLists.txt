set(CMAKE_CXX_STANDARD 23)

include_directories(../)



add_definitions(
    -DBRE_TEST
)

file(GLOB TEST_SOURCES
    tests/*.cpp
)

MESSAGE(STATUS "Test sources: ${TEST_SOURCES}")

add_boost_test(test_breutil
    ${TEST_SOURCES}
)

if(PLATFORM_LINUX)
    add_executable(benchmark
        tests/benchmark.cpp
    )
    
    target_include_directories(benchmark PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    target_link_libraries(benchmark PRIVATE
        Threads::Threads
    )
endif()


if(APPLE)
add_compile_definitions(_LIBCPP_ENABLE_EXPERIMENTAL)
target_link_libraries(base_test PRIVATE         
    "-framework AppKit"
    "-framework CoreVideo"
    "-framework IOKit"
    "-framework IOSurface"
    "-framework CoreMedia"
    "-framework CoreGraphics"
    "-framework AVFoundation"
    "-framework CoreFoundation"
    "-framework CoreServices"
    "-framework CoreAudio"
    "-framework SystemConfiguration"
)
endif()


copy_directory(${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR})