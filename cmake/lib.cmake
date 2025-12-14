# introduce
# boost, ffmpeg, sdl3, opencv, openssl, qt, spdlog, benchmark, zlib, breutils, pcap, librtp
# using link_xxx(target) to link the libraries


if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW)
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()


if(APPLE)
    include(cmake/macos.cmake)                  # macOS平台配置
elseif(UNIX)
    include(cmake/linux.cmake)                 # Linux平台配置
elseif(WIN32)
    include(cmake/windows.cmake)               # Windows平台配置
else()
    message(FATAL_ERROR "Unsupported platform. Please check your platform configuration file.")
endif()

set(LINK_FUNCTIONS_PATH "${CMAKE_SOURCE_DIR}/cmake/link_function")
include(${LINK_FUNCTIONS_PATH}/link_boost.cmake)
include(${LINK_FUNCTIONS_PATH}/link_openssl.cmake)
include(${LINK_FUNCTIONS_PATH}/link_ffmpeg.cmake)
include(${LINK_FUNCTIONS_PATH}/link_sdl3.cmake)
include(${LINK_FUNCTIONS_PATH}/link_opencv.cmake)
include(${LINK_FUNCTIONS_PATH}/link_qt.cmake)
include(${LINK_FUNCTIONS_PATH}/link_spdlog.cmake)
include(${LINK_FUNCTIONS_PATH}/link_benchmark.cmake)
include(${LINK_FUNCTIONS_PATH}/link_zlib.cmake)
include(${LINK_FUNCTIONS_PATH}/link_librtp.cmake)
include(${LINK_FUNCTIONS_PATH}/link_breutils.cmake)
include(${LINK_FUNCTIONS_PATH}/link_pcap.cmake)

