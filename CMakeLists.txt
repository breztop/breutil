cmake_minimum_required(VERSION 3.20)
project(breutil VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# 只有在编写本项目测试的时候才引用下面的 cmake/xxx, 外部项目不引用
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(BREUTIL_BUILD_TESTS ON CACHE BOOL "Build tests for breutil")
else()
    set(BREUTIL_BUILD_TESTS OFF CACHE BOOL "Build tests for breutil")
endif()

if(BUILD_DEB)
	set(BREUTIL_BUILD_TESTS OFF CACHE BOOL "Disable tests in deb build" FORCE)
endif()

if(BREUTIL_BUILD_TESTS)
    message(STATUS "Building tests for breutil")

	# 启用测试功能
	enable_testing()
    include(CTest)

	include(cmake/compiler.cmake)

	include(cmake/util.cmake)                     # 引入工具函数逻辑

    # include(cmake/thirdlib.cmake)               # 引入第三方库管理逻辑
    include(cmake/generate_build_info.cmake)      # 引入生成构建信息逻辑
    include(cmake/lib.cmake)                      # 引入库管理逻辑
    include(cmake/test.cmake)                     # 引入测试逻辑
    include(cmake/option.cmake)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Header-only 目标，供外部项目通过 find_package 引用
add_library(breutil INTERFACE)
add_library(breutil::breutil ALIAS breutil)

target_compile_features(breutil INTERFACE cxx_std_20)
target_include_directories(breutil
	INTERFACE
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
		$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# 安装头文件与目标导出
install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/breutil
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	PATTERN "*.h"
    PATTERN "*.hpp"
)

install(
	TARGETS breutil
	EXPORT breutilTargets
)

install(
	EXPORT breutilTargets
	FILE breutilTargets.cmake
	NAMESPACE breutil::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/breutil
)

configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/breutilConfig.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/breutilConfig.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/breutil
)

write_basic_package_version_file(
	${CMAKE_CURRENT_BINARY_DIR}/breutilConfigVersion.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion
)

install(
	FILES
		${CMAKE_CURRENT_BINARY_DIR}/breutilConfig.cmake
		${CMAKE_CURRENT_BINARY_DIR}/breutilConfigVersion.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/breutil
)

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
	IMMEDIATE
	@ONLY
)

add_custom_target(uninstall
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
	COMMENT "Uninstalling files listed in install_manifest.txt"
)