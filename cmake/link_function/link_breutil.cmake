# ========== 仅头文件库 ==========
function(link_breutil target)
    if(BREUTILS_DIR_PATH)
        target_include_directories(${target} PRIVATE ${BREUTILS_DIR_PATH})
        message(STATUS "Linked BREUTIL header files for target: ${target}")
        return()
    endif()

    if(NOT TARGET breutil::breutil)
        include(FetchContent)

        FetchContent_Declare(
            breutil
            GIT_REPOSITORY https://github.com/breztop/breutil.git
            GIT_TAG        main
        )

        FetchContent_MakeAvailable(breutil)

        add_library(breutil INTERFACE)
        target_include_directories(breutil INTERFACE ${breutil_SOURCE_DIR}/include)
        add_library(breutil::breutil ALIAS breutil)

        message(STATUS "breutil 已下载并添加到项目中")
    endif()

    target_link_libraries(${target} PRIVATE breutil::breutil)
endfunction()