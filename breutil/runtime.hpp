#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <psapi.h>
#include <tlhelp32.h>
#include <windows.h>
#pragma comment(lib, "psapi.lib")
#else
#include <dirent.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

// #include <stacktrace>


namespace bre::runtime {

// ---------- 编译时信息 ----------
constexpr const char* OS() {
#ifdef _WIN32
    return "windows";
#elif __linux__
    return "linux";
#elif __APPLE__
    return "darwin";
#elif __FreeBSD__
    return "freebsd";
#else
    return "unknown";
#endif
}

constexpr const char* ARCH() {
#if defined(__x86_64__) || defined(_M_X64)
    return "amd64";
#elif defined(__i386__) || defined(_M_IX86)
    return "386";
#elif defined(__aarch64__) || defined(_M_ARM64)
    return "arm64";
#elif defined(__arm__)
    return "arm";
#else
    return "unknown";
#endif
}


// ---------- 当前线程数 ----------
inline int NumGoroutine() {
#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 1;

    THREADENTRY32 te;
    te.dwSize = sizeof(te);
    int count = 0;
    if (Thread32First(snapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == pid) ++count;
        } while (Thread32Next(snapshot, &te));
    }
    CloseHandle(snapshot);
    return count;
#else
    // Linux: 读取 /proc/self/task 下的目录项数
    DIR* dir = opendir("/proc/self/task");
    if (!dir) return 1;
    int count = 0;
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_name[0] != '.') ++count;
    }
    closedir(dir);
    return count;
#endif
}

// ---------- 内存统计 ----------
struct MemStats {
    uint64_t Alloc;       // 当前已分配但未释放的字节数（近似 RSS）
    uint64_t TotalAlloc;  // 累计分配字节数（无法精确，设为 Alloc）
    uint64_t Sys;         // 从操作系统获取的内存（虚拟内存大小）
    uint64_t HeapAlloc;   // 同 Alloc
    uint64_t HeapSys;     // 堆系统内存（同 Sys）
    uint64_t HeapInuse;   // 使用中堆内存（同 Alloc）

    // uint64_t HeapIdle;      // 空闲堆内存（暂不支持，设为0）
    // uint64_t HeapReleased;  // 释放给 OS 的内存（暂不支持，设为0）
    // uint64_t HeapObjects;   // 堆上对象数（无法统计，设为0）
};

inline void ReadMemStats(MemStats* m) {
    if (!m) {
        return;
    }
    std::memset(m, 0, sizeof(MemStats));

#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        m->Alloc = pmc.WorkingSetSize;  // 常驻内存
        m->Sys = pmc.PagefileUsage;     // 虚拟内存
        m->HeapAlloc = pmc.WorkingSetSize;
        m->HeapSys = pmc.PagefileUsage;
        m->HeapInuse = pmc.WorkingSetSize;
        m->TotalAlloc = pmc.PeakWorkingSetSize;  // 近似峰值
    }
#else
    // Linux: 读取 /proc/self/statm
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        long size = 0, resident = 0, share = 0, text = 0, lib = 0, data = 0, dt = 0;
        statm >> size >> resident >> share >> text >> lib >> data >> dt;
        statm.close();
        long pageSize = sysconf(_SC_PAGESIZE);
        m->Sys = static_cast<uint64_t>(size) * pageSize;        // 虚拟内存
        m->Alloc = static_cast<uint64_t>(resident) * pageSize;  // 常驻内存
        m->HeapAlloc = m->Alloc;
        m->HeapSys = m->Sys;
        m->HeapInuse = m->Alloc;
        m->TotalAlloc = m->Alloc;  // 无法获取累计，暂用当前值
    }
#endif
}

// // 获取调用者的描述字符串，skip 表示跳过栈顶的帧数（0 表示调用 Caller 的函数）
// inline std::string Caller(int skip = 0) {
//     // 获取当前栈跟踪
//     std::stacktrace trace = std::stacktrace::current();
//     // 目标帧索引：第0帧是Caller自身，第1帧是Caller的调用者，所以需要跳过 skip+1 帧
//     std::size_t target = static_cast<std::size_t>(skip) + 1;
//     if (target >= trace.size()) {
//         return "?";
//     }
//     const auto& entry = trace[target];
//     std::string result;
//     if (std::string desc = entry.description(); !desc.empty()) {
//         result = desc;
//     } else {
//         result = "?";
//     }
//     // 添加源文件和行号
//     if (std::string file = entry.source_file(); !file.empty()) {
//         result += " at ";
//         result += file;
//         if (unsigned line = entry.source_line(); line != 0) {
//             result += ":";
//             result += std::to_string(line);
//         }
//     }
//     return result;
// }

// // 将当前线程的调用栈追加到 buf 中
// inline void Stack(std::string& buf) {
//     std::stacktrace trace = std::stacktrace::current();
//     if (trace.empty()) {
//         buf += "no stack\n";
//         return;
//     }
//     for (std::size_t i = 0; i < trace.size(); ++i) {
//         const auto& entry = trace[i];
//         buf += "#";
//         buf += std::to_string(i);
//         buf += "  ";
//         if (std::string desc = entry.description(); !desc.empty()) {
//             buf += desc;
//         } else {
//             buf += "?";
//         }
//         if (std::string file = entry.source_file(); !file.empty()) {
//             buf += " at ";
//             buf += file;
//             if (unsigned line = entry.source_line(); line != 0) {
//                 buf += ":";
//                 buf += std::to_string(line);
//             }
//         }
//         buf += "\n";
//     }
// }

}  // namespace bre::runtime
