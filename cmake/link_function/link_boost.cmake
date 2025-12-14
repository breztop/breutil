

function(link_boost_header target)
    # 查找 Boost（不需要任何组件）
    find_package(Boost REQUIRED)
    
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost 未找到！请检查路径或安装情况")
    endif()
    
    # 输出调试信息
    message(STATUS "Boost header-only found: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost version: ${Boost_VERSION}")
    
    # 只包含头文件目录，不链接任何库
    target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
endfunction()



function(link_boost target components)
    # 查找 Boost，指定需要链接的组件
    set(CMAKE_PREFIX_PATH ${BOOST_DIR_PATH})
    find_package(Boost REQUIRED COMPONENTS ${components})
    
    if(NOT Boost_FOUND)
        message(FATAL_ERROR "Boost not found! Please check the path or installation.")
    endif()
    
    # 输出调试信息
    message(STATUS "Boost found: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost version: ${Boost_VERSION}")
    message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
    message(STATUS "Boost components: ${components}")
    
    # 包含头文件目录
    target_include_directories(${target} PRIVATE ${Boost_INCLUDE_DIRS})
    
    # 链接需要编译的库
    target_link_libraries(${target} PRIVATE ${Boost_LIBRARIES})
endfunction()
