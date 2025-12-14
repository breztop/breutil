
# OpenSSL 链接函数
function(link_openssl target)
    if(NOT OPENSSL_DIR_PATH)
        message(FATAL_ERROR "OPENSSL_DIR_PATH未设置，请检查平台配置文件")
    endif()

    include_directories(${OPENSSL_DIR_PATH}/include)
    link_directories(${OPENSSL_DIR_PATH}/lib)

    set(OPENSSL_LIBS ssl crypto)
    foreach(lib ${OPENSSL_LIBS})
        find_library(${lib}_LIB ${lib} PATHS ${OPENSSL_DIR_PATH}/lib NO_DEFAULT_PATH)
        if(NOT ${lib}_LIB)
            message(FATAL_ERROR "找不到OpenSSL库: ${lib}")
        endif()
        list(APPEND OPENSSL_LIBRARIES ${${lib}_LIB})
    endforeach()
    target_link_libraries(${target} PRIVATE ${OPENSSL_LIBRARIES})
endfunction()