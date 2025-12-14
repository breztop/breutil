# Qt 链接函数
function(link_qt target Components)
    # 启用 AUTOMOC 和 AUTOUIC（处理 .ui 或 .qrc 文件）
    set_target_properties(${target} PROPERTIES
        AUTOMOC TRUE
        AUTOUIC TRUE
        AUTORCC TRUE
    )

    message(STATUS "link_qt(${target}) with components: ${Components}")

    if(NOT QTDIR)
        message(FATAL_ERROR "QTDIR未设置，请检查平台配置文件")
    endif()

    # 直接设置 Qt5_DIR 路径（比 CMAKE_PREFIX_PATH 更明确）
    set(Qt5_DIR "${QTDIR}/lib/cmake/Qt5" CACHE PATH "Qt5 directory" FORCE)

    # 解析传入的组件参数（将空格分隔的字符串转为列表）
    separate_arguments(Components_LIST UNIX_COMMAND "${Components}")

    # 确保包含 Core（Qt 必须的组件）
    list(APPEND Components_LIST Core)

    # 转换组件名称（如将 "GUI" 转为 "Gui"）
    foreach(component IN LISTS Components_LIST)
        string(TOUPPER ${component} component_upper)
        if(component_upper STREQUAL "GUI")
            list(APPEND Qt5_COMPONENTS Gui)
        else()
            list(APPEND Qt5_COMPONENTS ${component})
        endif()
    endforeach()

    # 查找 Qt5，并指定需要的组件
    find_package(Qt5 REQUIRED COMPONENTS ${Qt5_COMPONENTS})

    if(NOT Qt5_FOUND)
        message(FATAL_ERROR "Qt5 未找到！请检查路径或组件名称是否正确")
    endif()

    # 生成需要链接的 Qt 库列表
    set(Qt5_LIBRARIES "")
    foreach(component IN LISTS Qt5_COMPONENTS)
        list(APPEND Qt5_LIBRARIES Qt5::${component})
    endforeach()

    # 链接库（使用导入目标）
    target_link_libraries(${target} PRIVATE ${Qt5_LIBRARIES})

    # 输出调试信息
    message(STATUS "Qt5_DIR: ${Qt5_DIR}")
    message(STATUS "Qt5_COMPONENTS: ${Qt5_COMPONENTS}")
    message(STATUS "Qt5_LIBRARIES: ${Qt5_LIBRARIES}")
    message(STATUS "Qt5_VERSION: ${Qt5_VERSION}")
endfunction()