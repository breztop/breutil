#pragma once

// 用于在Windows上执行CMD命令并获取输出,不弹出窗口

#include <windows.h>

#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace bre {

/**
 * @brief 在不弹出窗口的情况下执行CMD命令并获取输出
 *
 * @param cmd 要执行的命令字符串
 * @param timeoutMs 超时时间(毫秒),默认30000ms(30秒),0表示无限等待
 * @param exitCode 输出参数,返回进程退出码,可以为nullptr
 * @return std::string 命令的标准输出和标准错误输出的合并结果
 *
 * @note 此函数会自动调用cmd.exe来执行命令
 * @note 如果命令执行失败或超时,返回空字符串
 *
 * @example
 * std::string output = execute_command_impl("ipconfig /all");
 * std::string output2 = execute_command_impl("dir C:\\", 5000);
 */
inline std::string execute_command_impl(const std::string& cmd, DWORD timeoutMs,
                                        DWORD* exitCode = nullptr) {
    // 初始化安全属性,允许句柄继承
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // 创建管道用于获取命令输出
    HANDLE hReadPipe = NULL, hWritePipe = NULL;
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        std::cerr << "CreatePipe failed, error: " << GetLastError() << std::endl;
        return "";
    }

    // 确保读取端句柄不被子进程继承
    if (!SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation failed, error: " << GetLastError() << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return "";
    }

    // 初始化启动信息结构体
    STARTUPINFOA si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;  // 隐藏窗口
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    // 初始化进程信息结构体
    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));

    // 构建完整的命令行: cmd.exe /c "命令"
    std::string fullCmd = "cmd.exe /c \"" + cmd + "\"";

    // CreateProcessA 需要可修改的字符串
    std::vector<char> cmdBuf(fullCmd.begin(), fullCmd.end());
    cmdBuf.push_back('\0');

    // 创建进程
    BOOL success = CreateProcessA(NULL,              // lpApplicationName
                                  cmdBuf.data(),     // lpCommandLine
                                  NULL,              // lpProcessAttributes
                                  NULL,              // lpThreadAttributes
                                  TRUE,              // bInheritHandles - 必须为TRUE以继承管道句柄
                                  CREATE_NO_WINDOW,  // dwCreationFlags - 不创建窗口
                                  NULL,              // lpEnvironment
                                  NULL,              // lpCurrentDirectory
                                  &si,               // lpStartupInfo
                                  &pi                // lpProcessInformation
    );

    if (!success) {
        DWORD error = GetLastError();
        std::cerr << "CreateProcessA failed, error: " << error << std::endl;
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return "";
    }

    // 关闭不需要的句柄
    CloseHandle(pi.hThread);
    CloseHandle(hWritePipe);  // 关闭父进程的写端,这样子进程结束后ReadFile会返回

    // 读取输出
    std::string output;
    output.reserve(1024);
    const DWORD bufferSize = 4096;
    char buffer[bufferSize];
    DWORD bytesRead = 0;
    BOOL readSuccess = FALSE;

    // 循环读取管道中的数据
    do {
        readSuccess = ReadFile(hReadPipe, buffer, bufferSize - 1, &bytesRead, NULL);
        if (readSuccess && bytesRead > 0) {
            buffer[bytesRead] = '\0';
            output.append(buffer, bytesRead);
        }
    } while (readSuccess && bytesRead > 0);

    // 等待进程结束
    DWORD waitResult = WaitForSingleObject(pi.hProcess, timeoutMs);

    if (waitResult == WAIT_TIMEOUT) {
        std::cerr << "Command execution timeout after " << timeoutMs << "ms" << std::endl;
        TerminateProcess(pi.hProcess, 1);  // 强制终止超时进程
        CloseHandle(hReadPipe);
        CloseHandle(pi.hProcess);
        return "";
    } else if (waitResult == WAIT_FAILED) {
        std::cerr << "WaitForSingleObject failed, error: " << GetLastError() << std::endl;
    }

    // 获取进程退出码
    DWORD processExitCode = 0;
    if (GetExitCodeProcess(pi.hProcess, &processExitCode)) {
        if (exitCode != nullptr) {
            *exitCode = processExitCode;
        }
        if (processExitCode != 0) {
            std::cerr << "Command exited with code: " << processExitCode << std::endl;
        }
    }

    // 清理资源
    CloseHandle(hReadPipe);
    CloseHandle(pi.hProcess);

    return output;
}

/**
 * @brief 在不弹出窗口的情况下执行CMD命令(简化版本,无超时限制)
 *
 * @param cmd 要执行的命令字符串
 * @param timeout_ms 超时时间(毫秒),默认3000ms(3秒),0表示无限等待
 * @return int 命令的退出码,0表示成功,非0表示失败
 */
inline int execute_command(const std::string& cmd, int timeout_ms = 3000) {
    DWORD exitCode = 0;
    execute_command_impl(cmd, timeout_ms, &exitCode);
    return exitCode;
}

/**
 * @brief 检查命令是否执行成功
 *
 * @param cmd 要执行的命令
 * @param output 输出参数,返回命令输出内容
 * @return int 命令是否执行成功(退出码为0)
 */
inline int execute_command(const std::string& cmd, std::string& output, int timeout_ms = 3000) {
    DWORD exitCode = 0;
    output = execute_command_impl(cmd, timeout_ms, &exitCode);
    return exitCode;
}

}  // namespace bre