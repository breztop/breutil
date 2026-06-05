#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace bre::os {

// 表示一个准备执行的外部命令（类似 exec.Cmd）
class Command {
public:
    // 构造函数：指定命令路径和参数
    explicit Command(const std::string& name, const std::vector<std::string>& args = {});
    ~Command();

    // ---------- 命令设置 ----------
    void SetArgs(const std::vector<std::string>& args);
    void SetEnv(const std::vector<std::string>& env);  // 格式 "KEY=VALUE"
    void SetDir(const std::string& dir);               // 工作目录
    void SetStdin(int fd);                             // 重定向标准输入
    void SetStdout(int fd);                            // 重定向标准输出
    void SetStderr(int fd);                            // 重定向标准错误

    // ---------- 执行方式 ----------
    // Run 启动命令并等待完成，返回退出码（成功返回 0）
    int Run();

    // Start 启动命令但不等待，返回 pid
    int Start();

    // Wait 等待已启动的命令结束，返回退出码
    int Wait();

    // Output 运行命令并返回标准输出的内容
    std::string Output();

    // CombinedOutput 运行命令并返回合并的标准输出和标准错误
    std::string CombinedOutput();

    // StdinPipe / StdoutPipe / StderrPipe 用于获取管道（可根据需要实现）

    // ---------- 进程状态 ----------
    int GetPid() const;
    bool IsRunning() const;

    // 类似 exec.Cmd 的 String() 方法
    std::string String() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

namespace exec {

// ---------- 辅助函数 ----------
// 在 PATH 中查找可执行文件（类似 exec.LookPath）
std::string LookPath(const std::string& file);

// 直接运行命令并等待（简化版）
int Run(const std::string& name, const std::vector<std::string>& args = {});

}  // namespace exec
}  // namespace bre::os
