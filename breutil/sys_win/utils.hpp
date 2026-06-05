#pragma once
#include <Windows.h>
#include <psapi.h>

#pragma comment(lib, "Kernel32.lib")
#pragma comment(lib, "Psapi.lib")

// 获取进程名称
inline std::string GetProcessName(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return "";
    }

    char processName[MAX_PATH] = {0};
    if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName))) {
        CloseHandle(hProcess);
        return std::string(processName);
    }

    CloseHandle(hProcess);
    return "";
}


// 获取进程路径
inline std::string GetProcessPath(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (hProcess == NULL) {
        return "";
    }

    char processPath[MAX_PATH] = {0};
    if (GetModuleFileNameExA(hProcess, NULL, processPath, sizeof(processPath))) {
        CloseHandle(hProcess);
        return std::string(processPath);
    }

    CloseHandle(hProcess);
    return "";
}
