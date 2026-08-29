set(CMAKE_CXX_STANDARD 23)

include_directories(../)

set(TEST_NAME test_breutil)

add_definitions(
    -DBRE_TEST
)

file(GLOB TEST_SOURCES
    tests/*.cpp
)

MESSAGE(STATUS "Test sources: ${TEST_SOURCES}")

add_boost_test(${TEST_NAME}
    ${TEST_SOURCES}
)

target_compile_definitions(${TEST_NAME} PRIVATE
    TEST_DATA_DIR="${TEST_DATA_PATH}"
)

# Compile the same conformance test with both Base64 implementations.
include(CheckIncludeFileCXX)
check_include_file_cxx("experimental/simd" BREUTIL_HAS_EXPERIMENTAL_SIMD)
foreach(BASE64_MODE IN ITEMS scalar simd)
    if(BASE64_MODE STREQUAL "simd" AND
       NOT BREUTIL_HAS_EXPERIMENTAL_SIMD)
        continue()
    endif()

    set(BASE64_TEST_NAME test_base64_${BASE64_MODE})
    add_executable(${BASE64_TEST_NAME} modes/base64_mode_test.cpp)
    target_include_directories(${BASE64_TEST_NAME} PRIVATE ${CMAKE_SOURCE_DIR})
    target_compile_definitions(${BASE64_TEST_NAME} PRIVATE
        BRE_BASE64_USE_SIMD=$<STREQUAL:${BASE64_MODE},simd>
    )
    add_test(NAME ${BASE64_TEST_NAME} COMMAND ${BASE64_TEST_NAME})
endforeach()

# set binary directory
set_target_properties(${TEST_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
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
target_link_libraries(${TEST_NAME} PRIVATE         
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

# same path within the binary directory
copy_directory(${CMAKE_CURRENT_SOURCE_DIR}/data ${CMAKE_CURRENT_BINARY_DIR})
