#pragma once

#if defined(__linux__) || defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include "sys_lin/execute_command_lin.hpp"
#elif defined(_WIN32)
#include "sys_win/execute_command_win.hpp"
#else
#error "Unsupported platform"
#endif


namespace bre {

/**
 * @brief 执行命令并获取输出，带超时限制
 * @param cmd 要执行的命令字符串
 * @param output 输出参数,返回命令输出内容
 * @param timeout_mili 超时时间(毫秒),默认3000ms(3秒),0表示无限等待
 * @return int 命令是否执行成功(退出码为0)
 */

inline int ExecuteCommand(const std::string& cmd, std::string& output, int timeout_mili = 3000) {
    return execute_command(cmd, output, timeout_mili);
}


/**
 * @brief 执行命令，带超时限制, 无需获取输出
 * @param cmd 要执行的命令字符串
 * @param timeout_mili 超时时间(毫秒),默认3000ms(3秒),0表示无限等待
 * @return int 命令是否执行成功(退出码为0)
 */
inline int ExecuteCommand(const std::string& cmd, int timeout_mili = 3000) {
    return execute_command(cmd, timeout_mili);
}


}  // namespace bre