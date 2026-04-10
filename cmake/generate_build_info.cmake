# 生成构建信息头文件的函数, 配合 generate_build_info.hpp 使用
# 参数:
#   target_name - 需要包含构建信息的目标名称
function(generate_build_info target_name)
    # 1. 编译时间
    string(TIMESTAMP BRE_BUILD_TIME "%Y-%m-%d %H:%M:%S")

    # 2. 初始化 Git 相关变量（默认值）
    set(BRE_GIT_COMMIT_HASH "unknown")
    set(BRE_GIT_BRANCH      "unknown")
    set(BRE_GIT_DIRTY       "false")
    set(BRE_GIT_TAG         "unknown")

    # 3. 尝试获取 Git 信息
    find_package(Git QUIET)
    if(GIT_FOUND AND EXISTS "${CMAKE_SOURCE_DIR}/.git")
        # 获取 short commit hash
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE BRE_GIT_COMMIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        # 获取当前分支名
        execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE BRE_GIT_BRANCH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )

        # 检查是否有未提交的修改（dirty）
        execute_process(
            COMMAND ${GIT_EXECUTABLE} status --porcelain
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE GIT_STATUS
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if(NOT "${GIT_STATUS}" STREQUAL "")
            set(BRE_GIT_DIRTY "true")
        endif()

        # 尝试获取最近的 tag（带 commit 距离，如 v1.2.0-3-gabc123）
        execute_process(
            COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty=""
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
            OUTPUT_VARIABLE BRE_GIT_TAG
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
        )
        if("${BRE_GIT_TAG}" STREQUAL "")
            set(BRE_GIT_TAG "unknown")
        endif()
    endif()

    # 4. 构建类型（Debug/Release 等）
    if(CMAKE_BUILD_TYPE)
        set(BRE_BUILD_TYPE "${CMAKE_BUILD_TYPE}")
    else()
        set(BRE_BUILD_TYPE "Unknown")
    endif()

    # 5. 编译器信息
    set(BRE_COMPILER_ID "${CMAKE_CXX_COMPILER_ID}")
    set(BRE_COMPILER_VERSION "${CMAKE_CXX_COMPILER_VERSION}")

    # 6. 项目版本（如果 project() 中指定了 VERSION）
    if(PROJECT_VERSION)
        set(BRE_PROJECT_VERSION "${PROJECT_VERSION}")
    else()
        set(BRE_PROJECT_VERSION "0.0.0")
    endif()

    # 7. 生成头文件路径（按目标命名避免冲突）
    set(BUILD_INFO_HEADER "${CMAKE_CURRENT_BINARY_DIR}/bre_build_info_${target_name}.h")

    # 8. 写入头文件
    file(WRITE "${BUILD_INFO_HEADER}"
"#pragma once

/* Auto-generated build info for target: ${target_name} */

#define BRE_BUILD_TIME          \"${BRE_BUILD_TIME}\"
#define BRE_GIT_COMMIT_HASH     \"${BRE_GIT_COMMIT_HASH}\"
#define BRE_GIT_BRANCH          \"${BRE_GIT_BRANCH}\"
#define BRE_GIT_DIRTY           ${BRE_GIT_DIRTY}
#define BRE_GIT_TAG             \"${BRE_GIT_TAG}\"
#define BRE_BUILD_TYPE          \"${BRE_BUILD_TYPE}\"
#define BRE_COMPILER_ID         \"${BRE_COMPILER_ID}\"
#define BRE_COMPILER_VERSION    \"${BRE_COMPILER_VERSION}\"
#define BRE_PROJECT_VERSION     \"${BRE_PROJECT_VERSION}\"
")

    # 9. 添加 include 路径
    target_include_directories(${target_name} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

    # （可选）定义一个宏表示构建信息已启用，便于 C++ 代码条件编译
    target_compile_definitions(${target_name} PRIVATE BRE_BUILD_INFO_HEADER="${BUILD_INFO_HEADER}")
endfunction()