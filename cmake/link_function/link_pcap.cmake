# pcap 链接函数
function(link_pcap target)
    if(APPLE)
        # macOS 自带 pcap 库，直接链接系统库
        find_library(PCAP_LIBRARY pcap)
        if(NOT PCAP_LIBRARY)
            message(FATAL_ERROR "pcap 库未找到，请确认系统是否安装了 libpcap")
        endif()
        
        message(STATUS "Found pcap library: ${PCAP_LIBRARY}")
        target_link_libraries(${target} PRIVATE ${PCAP_LIBRARY})
        
    elseif(UNIX)
        # Linux 系统使用 pkg-config 或 find_library
        find_library(PCAP_LIBRARY pcap)
        if(NOT PCAP_LIBRARY)
            message(FATAL_ERROR "pcap 库未找到，请安装 libpcap-dev")
        endif()
        
        message(STATUS "Found pcap library: ${PCAP_LIBRARY}")
        target_link_libraries(${target} PRIVATE ${PCAP_LIBRARY})
        
    elseif(WIN32)
        # Windows 需要安装 WinPcap 或 Npcap
        if(NOT PCAP_DIR_PATH)
            message(FATAL_ERROR "PCAP_DIR_PATH未设置，请在 windows.cmake 中设置 WinPcap/Npcap 路径")
        endif()
        
        include_directories(${PCAP_DIR_PATH}/include)
        link_directories(${PCAP_DIR_PATH}/lib)
        
        find_library(PCAP_LIBRARY NAMES wpcap PATHS ${PCAP_DIR_PATH}/lib NO_DEFAULT_PATH)
        if(NOT PCAP_LIBRARY)
            message(FATAL_ERROR "pcap 库未找到")
        endif()
        
        target_link_libraries(${target} PRIVATE ${PCAP_LIBRARY} ws2_32)
    else()
        message(FATAL_ERROR "不支持的平台，无法链接 pcap")
    endif()
endfunction()