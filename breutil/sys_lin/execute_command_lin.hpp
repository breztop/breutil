#pragma once

#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>


inline int execute_command(const std::string& cmd, std::string& output, int timeout_mili) {
    if (cmd.empty()) {
        std::cerr << "Empty command\n";
        return -1;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        std::cerr << "pipe() failed: " << std::strerror(errno) << "\n";
        return -1;
    }

    pid_t pid = fork();
    if (pid == -1) {
        std::cerr << "fork() failed: " << std::strerror(errno) << "\n";
        close(pipefd[0]);
        close(pipefd[1]);
        return -1;
    }

    if (pid == 0) {
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);

        execl("/bin/sh", "sh", "-c", cmd.c_str(), nullptr);

        std::exit(127);
    } else {
        close(pipefd[1]);

        // 设置管道读端为非阻塞
        int flags = fcntl(pipefd[0], F_GETFL, 0);
        fcntl(pipefd[0], F_SETFL, flags | O_NONBLOCK);

        output.clear();
        char buffer[4096];
        int total_wait = 0;
        const int poll_interval_ms = 50;  // 每 50ms 检查一次

        while (true) {
            int status;
            pid_t result = waitpid(pid, &status, WNOHANG);
            if (result == pid) {
                // 子进程已结束
                // 读取剩余数据
                ssize_t n;
                while ((n = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
                    output.append(buffer, n);
                }
                close(pipefd[0]);

                if (WIFEXITED(status)) {
                    int exit_code = WEXITSTATUS(status);
                    if (exit_code != 0) {
                        std::cerr << "Command failed with exit code: " << exit_code << "\n";
                        return exit_code;
                    }
                    return exit_code;
                } else {
                    std::cerr << "Command terminated by signal\n";
                    return -1;
                }
            }

            if (timeout_mili > 0 && total_wait >= timeout_mili) {
                kill(pid, SIGTERM);
                // 等待最多 100ms 让其优雅退出
                for (int i = 0; i < 2; ++i) {
                    usleep(50000);  // 50ms
                    if (waitpid(pid, &status, WNOHANG) == pid) {
                        break;
                    }
                }
                // 若仍未退出，强制杀死
                kill(pid, SIGKILL);
                waitpid(pid, nullptr, 0);  // 回收僵尸进程
                close(pipefd[0]);
                std::cerr << "Command timed out after " << timeout_mili << " ms: " << cmd << "\n";
                return -1;
            }

            // 用 select 等待可读（带超时）
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(pipefd[0], &readfds);

            struct timeval tv;
            tv.tv_sec = 0;
            tv.tv_usec = poll_interval_ms * 1000;  // 转为微秒

            int activity = select(pipefd[0] + 1, &readfds, nullptr, nullptr, &tv);
            if (activity > 0 && FD_ISSET(pipefd[0], &readfds)) {
                ssize_t n = read(pipefd[0], buffer, sizeof(buffer));
                if (n > 0) {
                    output.append(buffer, n);
                }
            }

            total_wait += poll_interval_ms;
        }
    }
}

inline int execute_command(const std::string& cmd, int timeout_mili) {
    std::string output;
    return execute_command(cmd, output, timeout_mili);
}