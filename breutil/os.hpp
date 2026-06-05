#pragma once


#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <map>
#include <regex>
#include <set>
#include <stdexcept>
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
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <sys/param.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
extern char** environ;
#endif

namespace bre::os {


int GetPID();
int GetPPID();
int GetUID();
int GetGID();
std::string Getwd();
std::string TempDir();
std::string Executable();
std::vector<std::string>& Args();
void InitArgs(int argc, char* argv[]);
std::string Getenv(const std::string& key);
bool LookupEnv(const std::string& key, std::string* value);
std::map<std::string, std::string> Environ();
void Setenv(const std::string& key, const std::string& value);
void Unsetenv(const std::string& key);
std::string Hostname();
int Getpagesize();

struct HWInfo;

std::string GetCpuVendor();
std::string GetCpuModel();
int GetCpuCores();
int GetCpuThreads();
std::string GetGpuVendor();
std::string GetGpuModel();
int GetGpuMemoryMB();


// ---------- 辅助函数：UTF-8 与宽字符转换 (Windows) ----------
#ifdef _WIN32
static inline std::string WStringToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), nullptr,
                                  0, nullptr, nullptr);
    std::string result(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.data(), static_cast<int>(wstr.size()), result.data(), len,
                        nullptr, nullptr);
    return result;
}

static inline std::wstring UTF8ToWString(const std::string& str) {
    if (str.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), nullptr, 0);
    std::wstring result(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.data(), static_cast<int>(str.size()), result.data(), len);
    return result;
}
#endif

// ---------- 进程信息 ----------
inline int GetPID() {
#ifdef _WIN32
    return static_cast<int>(GetCurrentProcessId());
#else
    return static_cast<int>(getpid());
#endif
}

inline int GetPPID() {
#ifdef _WIN32
    DWORD pid = GetCurrentProcessId();
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) return 0;
    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);
    DWORD ppid = 0;
    if (Process32First(snapshot, &pe)) {
        do {
            if (pe.th32ProcessID == pid) {
                ppid = pe.th32ParentProcessID;
                break;
            }
        } while (Process32Next(snapshot, &pe));
    }
    CloseHandle(snapshot);
    return static_cast<int>(ppid);
#else
    return static_cast<int>(getppid());
#endif
}

inline int GetUID() {
#ifdef _WIN32
    return -1;  // Windows 无有效 UID
#else
    return static_cast<int>(getuid());
#endif
}

inline int GetGID() {
#ifdef _WIN32
    return -1;
#else
    return static_cast<int>(getgid());
#endif
}

// ---------- 文件与路径 ----------
inline std::string Getwd() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    if (GetCurrentDirectoryW(MAX_PATH, buf) == 0)
        throw std::runtime_error("Getwd: GetCurrentDirectory failed");
    return WStringToUTF8(buf);
#else
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == nullptr) throw std::runtime_error("Getwd: getcwd failed");
    return std::string(buf);
#endif
}

inline std::string TempDir() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    DWORD len = GetTempPathW(MAX_PATH, buf);
    if (len == 0) throw std::runtime_error("TempDir: GetTempPath failed");
    std::string path = WStringToUTF8(buf);
    // 确保路径以分隔符结尾
    if (!path.empty() && path.back() != '\\' && path.back() != '/') path += '\\';
    return path;
#else
    const char* dir = getenv("TMPDIR");
    if (dir != nullptr && dir[0] != '\0') return dir;
    return "/tmp/";
#endif
}

inline std::string Executable() {
#ifdef _WIN32
    wchar_t buf[MAX_PATH];
    if (GetModuleFileNameW(nullptr, buf, MAX_PATH) == 0)
        throw std::runtime_error("Executable: GetModuleFileName failed");
    return WStringToUTF8(buf);
#else
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len == -1) {
        // fallback: 从 argv[0] 获取（需要用户设置 Args）
        // 如果没有设置 Args，抛异常
        throw std::runtime_error("Executable: readlink /proc/self/exe failed");
    }
    buf[len] = '\0';
    return std::string(buf);
#endif
}


inline std::vector<std::string>& Args() {
    static std::vector<std::string> args;
    return args;
}


inline void InitArgs(int argc, char* argv[]) {
    auto& args = Args();
    args.clear();
    for (int i = 0; i < argc; ++i) {
        args.emplace_back(argv[i]);
    }
}

// ---------- 环境变量 ----------
inline std::string Getenv(const std::string& key) {
#ifdef _WIN32
    std::wstring wkey = UTF8ToWString(key);
    wchar_t* val = nullptr;
    size_t len = 0;
    _wgetenv_s(&len, nullptr, 0, wkey.c_str());  // 获取所需长度
    if (len == 0) return {};
    std::wstring wval(len, L'\0');
    _wgetenv_s(&len, wval.data(), len, wkey.c_str());
    wval.resize(len - 1);  // 移除末尾 null
    return WStringToUTF8(wval);
#else
    const char* val = std::getenv(key.c_str());
    return val ? std::string(val) : std::string();
#endif
}

inline bool LookupEnv(const std::string& key, std::string* value) {
    std::string v = Getenv(key);
    if (value) *value = v;
    return !v.empty();
}

inline std::map<std::string, std::string> Environ() {
    std::map<std::string, std::string> env;
#ifdef _WIN32
    wchar_t* envstrings = GetEnvironmentStringsW();
    if (!envstrings) return env;
    for (wchar_t* p = envstrings; *p; p += wcslen(p) + 1) {
        std::wstring entry(p);
        size_t eq = entry.find(L'=');
        if (eq != std::wstring::npos) {
            std::string key = WStringToUTF8(entry.substr(0, eq));
            std::string value = WStringToUTF8(entry.substr(eq + 1));
            env[key] = value;
        }
    }
    FreeEnvironmentStringsW(envstrings);
#else
    for (char** e = environ; *e; ++e) {
        std::string entry(*e);
        size_t eq = entry.find('=');
        if (eq != std::string::npos) {
            std::string key = entry.substr(0, eq);
            std::string value = entry.substr(eq + 1);
            env[key] = value;
        }
    }
#endif
    return env;
}

inline void Setenv(const std::string& key, const std::string& value) {
#ifdef _WIN32
    std::wstring wkey = UTF8ToWString(key);
    std::wstring wval = UTF8ToWString(value);
    if (SetEnvironmentVariableW(wkey.c_str(), wval.c_str()) == 0)
        throw std::runtime_error("Setenv: SetEnvironmentVariable failed");
#else
    if (setenv(key.c_str(), value.c_str(), 1) != 0) {
        throw std::runtime_error("Setenv: setenv failed");
    }
#endif
}

inline void Unsetenv(const std::string& key) {
#ifdef _WIN32
    std::wstring wkey = UTF8ToWString(key);
    if (SetEnvironmentVariableW(wkey.c_str(), nullptr) == 0) {
        throw std::runtime_error("Unsetenv: SetEnvironmentVariable failed");
    }
#else
    if (unsetenv(key.c_str()) != 0) {
        throw std::runtime_error("Unsetenv: unsetenv failed");
    }
#endif
}

// ---------- 系统信息 ----------
inline std::string Hostname() {
#ifdef _WIN32
    wchar_t buf[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD len = sizeof(buf) / sizeof(wchar_t);
    if (!GetComputerNameW(buf, &len)) {
        throw std::runtime_error("Hostname: GetComputerName failed");
    }
    return WStringToUTF8(std::wstring(buf, len));
#else
    char buf[HOST_NAME_MAX];
    if (gethostname(buf, sizeof(buf)) != 0) {
        throw std::runtime_error("Hostname: gethostname failed");
    }
    return std::string(buf);
#endif
}

inline int Getpagesize() {
#ifdef _WIN32
    SYSTEM_INFO si;
    GetSystemInfo(&si);
    return static_cast<int>(si.dwPageSize);
#else
    return static_cast<int>(sysconf(_SC_PAGESIZE));
#endif
}


struct HWInfo {
    std::string cpu_vendor;
    std::string cpu_model;
    int cpu_cores;
    int cpu_threads;

    std::string gpu_vendor;
    std::string gpu_model;
    int gpu_memory_mb;

    HWInfo() {}

private:
    void detect() {
        cpu_vendor = GetCpuVendor();
        cpu_model = GetCpuModel();
        cpu_cores = GetCpuCores();
        cpu_threads = GetCpuThreads();

        gpu_vendor = GetGpuVendor();
        gpu_model = GetGpuModel();
        gpu_memory_mb = GetGpuMemoryMB();
    }
};


inline std::string GetCpuVendor() {
    std::string vendor = "Unknown";

#ifdef __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("vendor_id") == 0 || line.find("vendor") == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    vendor = line.substr(colon + 1);
                    // 去除首尾空白字符
                    vendor.erase(0, vendor.find_first_not_of(" \t"));
                    vendor.erase(vendor.find_last_not_of(" \t") + 1);
                    break;
                }
            }
        }
        cpuinfo.close();
    }
#endif
    return vendor;
}


inline std::string GetCpuModel() {
    std::string model = "Unknown";

#ifdef __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        while (std::getline(cpuinfo, line)) {
            if (line.find("model name") == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    model = line.substr(colon + 1);
                    // 去除首尾空白字符
                    model.erase(0, model.find_first_not_of(" \t"));
                    model.erase(model.find_last_not_of(" \t") + 1);
                    break;
                }
            }
        }
        cpuinfo.close();
    }
#endif
    return model;
}

inline int GetCpuCores() {
    int cores = static_cast<int>(std::thread::hardware_concurrency());
    if (cores > 0) {
        return cores;
    }

#ifdef __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    if (cpuinfo.is_open()) {
        std::string line;
        std::set<int> physical_ids;

        while (std::getline(cpuinfo, line)) {
            if (line.find("physical id") == 0) {
                size_t colon = line.find(':');
                if (colon != std::string::npos) {
                    std::string id_str = line.substr(colon + 1);
                    id_str.erase(0, id_str.find_first_not_of(" \t"));
                    id_str.erase(id_str.find_last_not_of(" \t") + 1);
                    int id = std::stoi(id_str);
                    physical_ids.insert(id);
                }
            }
        }
        cpuinfo.close();

        if (!physical_ids.empty()) {
            // 重新打开文件统计每个物理CPU的核心数
            cpuinfo.open("/proc/cpuinfo");
            std::map<int, int> cores_per_physical;
            int current_physical_id = -1;
            int current_core_id = -1;

            while (std::getline(cpuinfo, line)) {
                if (line.find("physical id") == 0) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string id_str = line.substr(colon + 1);
                        id_str.erase(0, id_str.find_first_not_of(" \t"));
                        id_str.erase(id_str.find_last_not_of(" \t") + 1);
                        current_physical_id = std::stoi(id_str);
                    }
                } else if (line.find("core id") == 0) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        std::string id_str = line.substr(colon + 1);
                        id_str.erase(0, id_str.find_first_not_of(" \t"));
                        id_str.erase(id_str.find_last_not_of(" \t") + 1);
                        current_core_id = std::stoi(id_str);

                        if (current_physical_id >= 0 && current_core_id >= 0) {
                            cores_per_physical[current_physical_id] = std::max(
                                cores_per_physical[current_physical_id], current_core_id + 1);
                        }
                    }
                }
            }
            cpuinfo.close();

            // 计算总核心数
            cores = 0;
            for (const auto& pair : cores_per_physical) {
                cores += pair.second;
            }
        }
    }
#endif

    if (cores <= 0) {
        cores = 1;  // 默认值
    }

    return cores;
}

inline int GetCpuThreads() {
    int threads = 0;

#ifdef __linux__
    // 获取逻辑CPU数量
    threads = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));

    if (threads <= 0) {
        // 备用方法：读取/proc/cpuinfo
        std::ifstream cpuinfo("/proc/cpuinfo");
        if (cpuinfo.is_open()) {
            std::string line;
            while (std::getline(cpuinfo, line)) {
                if (line.find("processor") == 0) {
                    threads++;
                }
            }
            cpuinfo.close();
        }
    }
#endif

    if (threads <= 0) {
        threads = static_cast<int>(std::thread::hardware_concurrency());
    }

    return threads;
}

inline std::string GetGpuVendor() {
    std::string vendor = "Unknown";

#ifdef __linux__
    // 尝试检测NVIDIA GPU
    std::ifstream nvidia_check("/proc/driver/nvidia/version");
    if (nvidia_check.is_open()) {
        vendor = "NVIDIA";
        nvidia_check.close();
    } else {
        // 尝试通过PCI信息检测
        std::ifstream pci_ids("/usr/share/misc/pci.ids");
        if (!pci_ids.is_open()) {
            pci_ids.open("/usr/share/pci.ids");
        }

        if (pci_ids.is_open()) {
            std::string line;
            std::regex vendor_regex(R"(^\s*([0-9a-fA-F]{4})\s+(.+)$)");
            std::regex device_regex(R"(^\s*([0-9a-fA-F]{4})\s+(.+)$)");

            while (std::getline(pci_ids, line)) {
                std::smatch match;
                if (std::regex_match(line, match, vendor_regex) && match.size() == 3) {
                    std::string vendor_id = match[1];
                    if (vendor_id == "10de") {         // NVIDIA
                        vendor = "NVIDIA";
                        break;
                    } else if (vendor_id == "1002") {  // AMD
                        vendor = "AMD";
                        break;
                    } else if (vendor_id == "8086") {  // Intel
                        vendor = "Intel";
                        break;
                    }
                }
            }
            pci_ids.close();
        }
    }

    // 如果没有检测到，尝试通过lspci命令
    if (vendor == "Unknown") {
        FILE* pipe = popen("lspci 2>/dev/null | grep -i vga 2>/dev/null | head -1", "r");
        if (pipe) {
            char buffer[128];
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                std::string output(buffer);
                if (output.find("NVIDIA") != std::string::npos) {
                    vendor = "NVIDIA";
                } else if (output.find("AMD") != std::string::npos ||
                           output.find("ATI") != std::string::npos) {
                    vendor = "AMD";
                } else if (output.find("Intel") != std::string::npos) {
                    vendor = "Intel";
                }
            }
            pclose(pipe);
        }
    }
#endif
    return vendor;
}

inline std::string GetGpuModel() {
    std::string model = "Unknown";

#ifdef __linux__
    // 尝试通过lspci获取GPU型号
    FILE* pipe = popen("lspci 2>/dev/null | grep -i vga 2>/dev/null | head -1", "r");
    if (pipe) {
        char buffer[256];
        if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            std::string output(buffer);
            // 提取型号信息（通常在:后面的部分）
            size_t colon = output.find(':');
            if (colon != std::string::npos) {
                model = output.substr(colon + 1);
                // 去除行尾换行符
                if (!model.empty() && model[model.size() - 1] == '\n') {
                    model.pop_back();
                }
                // 去除首尾空白字符
                model.erase(0, model.find_first_not_of(" \t"));
                model.erase(model.find_last_not_of(" \t") + 1);
            }
        }
        pclose(pipe);
    }

    // 如果没有获取到，尝试通过NVIDIA驱动获取
    if (model == "Unknown" || model.empty()) {
        std::ifstream nvidia_model("/proc/driver/nvidia/gpus/0/information");
        if (nvidia_model.is_open()) {
            std::string line;
            while (std::getline(nvidia_model, line)) {
                if (line.find("Model:") != std::string::npos) {
                    size_t colon = line.find(':');
                    if (colon != std::string::npos) {
                        model = line.substr(colon + 1);
                        model.erase(0, model.find_first_not_of(" \t"));
                        model.erase(model.find_last_not_of(" \t") + 1);
                        break;
                    }
                }
            }
            nvidia_model.close();
        }
    }
#endif

    return model;
}

inline int GetGpuMemoryMB() {
    int memory_mb = 0;

#ifdef __linux__
    // 尝试通过NVIDIA驱动获取显存
    std::string vendor = GetGpuVendor();

    if (vendor == "NVIDIA") {
        // 尝试通过nvidia-smi获取显存
        FILE* pipe = popen(
            "nvidia-smi --query-gpu=memory.total --format=csv,noheader,nounits 2>/dev/null", "r");
        if (pipe) {
            char buffer[32];
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                memory_mb = std::stoi(buffer);
            }
            pclose(pipe);
        }
    } else if (vendor == "AMD") {
        // 尝试通过radeontop或rocm-smi获取显存
        FILE* pipe = popen(
            "rocm-smi --showmeminfo vram 2>/dev/null | grep 'Total Memory' | awk '{print "
            "$3}'",
            "r");
        if (pipe) {
            char buffer[32];
            if (fgets(buffer, sizeof(buffer), pipe) != NULL) {
                memory_mb = std::stoi(buffer) / 1024;  // 转换为MB
            }
            pclose(pipe);
        }
    } else if (vendor == "Intel") {
        // Intel集成显卡通常共享系统内存
        memory_mb = 0;  // 设置为0表示共享内存
    }

    // 如果没有获取到，尝试通用方法
    if (memory_mb <= 0) {
        // 尝试通过/proc/mtrr或sysfs获取
        std::ifstream meminfo("/proc/meminfo");
        if (meminfo.is_open()) {
            std::string line;
            int total_mem = 0;
            while (std::getline(meminfo, line)) {
                if (line.find("MemTotal:") == 0) {
                    std::istringstream iss(line.substr(9));
                    iss >> total_mem;  // 单位是KB
                    break;
                }
            }
            meminfo.close();

            // 对于集成显卡，假设分配了1/4的系统内存
            memory_mb = total_mem / 4 / 1024;
        }
    }
#endif
    return memory_mb;
}


}  // namespace bre::os
