# OpenCV 链接函数
function(link_opencv target)
    find_package(OpenCV REQUIRED)
    target_link_libraries(${target} PRIVATE ${OpenCV_LIBS})
endfunction()