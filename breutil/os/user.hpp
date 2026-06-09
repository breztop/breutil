#pragma once

#include <cstring>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <lm.h>
#include <sddl.h>
#include <windows.h>
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "advapi32.lib")
#else
#include <grp.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace bre::os::user {

struct Info {
    std::string Uid;
    std::string Gid;
    std::string Username;
    std::string Name;  // 全名（POSIX 下为 gecos 字段，Windows 下为用户全名）
    std::string HomeDir;
};

struct Group {
    std::string Gid;
    std::string Name;
};

// 获取当前用户信息
inline Info Current();

// 根据 UID 字符串查找用户（如 "1000" 或 SID 字符串）
inline Info LookupId(const std::string& uid);

// 根据用户名查找用户
inline Info Lookup(const std::string& username);

// 根据 GID 字符串查找组
inline Group LookupGroup(const std::string& gid);

// 根据组名查找组
inline Group LookupGroupName(const std::string& name);

// -------------------------------------------------------------------
// 实现部分
// -------------------------------------------------------------------

#ifdef _WIN32

// 将 SID 转换为字符串
static inline std::string SidToString(PSID sid) {
    LPWSTR sidStrW = nullptr;
    if (!ConvertSidToStringSidW(sid, &sidStrW)) {
        throw std::runtime_error("ConvertSidToStringSid failed");
    }
    char sidStr[256];
    WideCharToMultiByte(CP_UTF8, 0, sidStrW, -1, sidStr, sizeof(sidStr), nullptr, nullptr);
    LocalFree(sidStrW);
    return std::string(sidStr);
}

// 根据用户名获取用户信息（Windows 实现）
static inline Info getWindowsUserInfo(const std::string& username) {
    // 转换为宽字符
    std::vector<wchar_t> wuser(username.size() + 1);
    MultiByteToWideChar(CP_UTF8, 0, username.c_str(), -1, wuser.data(),
                        static_cast<int>(wuser.size()));

    // 获取用户信息
    LPUSER_INFO_4 pUserInfo = nullptr;
    DWORD dwLevel = 4;
    NET_API_STATUS status = NetUserGetInfo(nullptr, wuser.data(), dwLevel, (LPBYTE*)&pUserInfo);
    if (status != NERR_Success) {
        throw std::runtime_error("NetUserGetInfo failed: user not found");
    }

    Info info;
    // UID: SID 字符串
    info.Uid = SidToString(pUserInfo->usri4_user_sid);
    // GID: 主组的 SID
    info.Gid = SidToString(pUserInfo->usri4_primary_group_id);
    // 用户名
    char buf[256];
    WideCharToMultiByte(CP_UTF8, 0, pUserInfo->usri4_name, -1, buf, sizeof(buf), nullptr, nullptr);
    info.Username = buf;
    // 全名
    WideCharToMultiByte(CP_UTF8, 0, pUserInfo->usri4_full_name, -1, buf, sizeof(buf), nullptr,
                        nullptr);
    info.Name = buf;
    // 主目录
    WideCharToMultiByte(CP_UTF8, 0, pUserInfo->usri4_home_dir, -1, buf, sizeof(buf), nullptr,
                        nullptr);
    info.HomeDir = buf;

    NetApiBufferFree(pUserInfo);
    return info;
}

// 获取当前用户信息（Windows）
inline Info Current() {
    // 获取当前进程的用户令牌
    HANDLE hToken;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        throw std::runtime_error("OpenProcessToken failed");
    }

    DWORD size = 0;
    GetTokenInformation(hToken, TokenUser, nullptr, 0, &size);
    std::vector<char> buffer(size);
    PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());
    if (!GetTokenInformation(hToken, TokenUser, pTokenUser, size, &size)) {
        CloseHandle(hToken);
        throw std::runtime_error("GetTokenInformation failed");
    }

    // 获取用户名
    char username[256];
    DWORD usernameLen = sizeof(username);
    if (!LookupAccountSidA(nullptr, pTokenUser->User.Sid, username, &usernameLen, nullptr, nullptr,
                           nullptr)) {
        CloseHandle(hToken);
        throw std::runtime_error("LookupAccountSid failed");
    }

    CloseHandle(hToken);
    return getWindowsUserInfo(username);
}

inline Info LookupId(const std::string& uid) {
    // Windows 下 UID 是 SID 字符串，不能直接转换，需要通过 SID 查找用户名
    PSID sid;
    std::vector<wchar_t> wsid(uid.size() + 1);
    MultiByteToWideChar(CP_UTF8, 0, uid.c_str(), -1, wsid.data(), static_cast<int>(wsid.size()));
    if (!ConvertStringSidToSidW(wsid.data(), &sid)) {
        throw std::runtime_error("Invalid SID string");
    }

    char username[256];
    DWORD usernameLen = sizeof(username);
    char domain[256];
    DWORD domainLen = sizeof(domain);
    SID_NAME_USE sidType;
    if (!LookupAccountSidA(nullptr, sid, username, &usernameLen, domain, &domainLen, &sidType)) {
        LocalFree(sid);
        throw std::runtime_error("LookupAccountSid failed: SID not found");
    }
    LocalFree(sid);

    return getWindowsUserInfo(username);
}

inline Info Lookup(const std::string& username) { return getWindowsUserInfo(username); }

inline Group LookupGroup(const std::string& gid) {
    // Windows 下 GID 是组 SID 字符串
    PSID sid;
    std::vector<wchar_t> wsid(gid.size() + 1);
    MultiByteToWideChar(CP_UTF8, 0, gid.c_str(), -1, wsid.data(), static_cast<int>(wsid.size()));
    if (!ConvertStringSidToSidW(wsid.data(), &sid)) {
        throw std::runtime_error("Invalid SID string");
    }

    char groupName[256];
    DWORD nameLen = sizeof(groupName);
    char domain[256];
    DWORD domainLen = sizeof(domain);
    SID_NAME_USE sidType;
    if (!LookupAccountSidA(nullptr, sid, groupName, &nameLen, domain, &domainLen, &sidType)) {
        LocalFree(sid);
        throw std::runtime_error("LookupAccountSid failed: group not found");
    }
    LocalFree(sid);

    Group grp;
    grp.Gid = gid;
    grp.Name = groupName;
    return grp;
}

inline Group LookupGroupName(const std::string& name) {
    // 通过组名查找组信息
    std::vector<wchar_t> wname(name.size() + 1);
    MultiByteToWideChar(CP_UTF8, 0, name.c_str(), -1, wname.data(), static_cast<int>(wname.size()));

    DWORD sidSize = 0, domainSize = 0;
    SID_NAME_USE sidType;
    LookupAccountNameW(nullptr, wname.data(), nullptr, &sidSize, nullptr, &domainSize, &sidType);
    if (sidSize == 0) {
        throw std::runtime_error("LookupAccountName failed: group not found");
    }

    std::vector<char> sidBuf(sidSize);
    std::vector<wchar_t> domainBuf(domainSize);
    if (!LookupAccountNameW(nullptr, wname.data(), sidBuf.data(), &sidSize, domainBuf.data(),
                            &domainSize, &sidType)) {
        throw std::runtime_error("LookupAccountName failed");
    }

    Group grp;
    grp.Gid = SidToString(reinterpret_cast<PSID>(sidBuf.data()));
    grp.Name = name;
    return grp;
}

#else   // POSIX (Linux/Unix)

// 将 passwd 结构填充到 Info
static inline Info fillInfoFromPasswd(const struct passwd* pwd) {
    Info info;
    info.Uid = std::to_string(pwd->pw_uid);
    info.Gid = std::to_string(pwd->pw_gid);
    info.Username = pwd->pw_name;
    info.Name = pwd->pw_gecos;
    info.HomeDir = pwd->pw_dir;
    return info;
}

// 获取当前用户
inline Info Current() {
    uid_t uid = getuid();
    struct passwd pwd;
    struct passwd* result = nullptr;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;
    std::vector<char> buffer(bufsize);

    int ret = getpwuid_r(uid, &pwd, buffer.data(), bufsize, &result);
    if (ret != 0 || !result) {
        throw std::runtime_error("getpwuid_r failed: user not found");
    }
    return fillInfoFromPasswd(&pwd);
}

// 根据 UID 字符串查找
inline Info LookupId(const std::string& uid) {
    uid_t uid_num = static_cast<uid_t>(std::stoul(uid));
    struct passwd pwd;
    struct passwd* result = nullptr;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;
    std::vector<char> buffer(bufsize);

    int ret = getpwuid_r(uid_num, &pwd, buffer.data(), bufsize, &result);
    if (ret != 0 || !result) {
        throw std::runtime_error("getpwuid_r failed: user not found");
    }
    return fillInfoFromPasswd(&pwd);
}

// 根据用户名查找
inline Info Lookup(const std::string& username) {
    struct passwd pwd;
    struct passwd* result = nullptr;
    long bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;
    std::vector<char> buffer(bufsize);

    int ret = getpwnam_r(username.c_str(), &pwd, buffer.data(), bufsize, &result);
    if (ret != 0 || !result) {
        throw std::runtime_error("getpwnam_r failed: user not found");
    }
    return fillInfoFromPasswd(&pwd);
}

// 填充 group 结构
static inline Group fillGroupFromGrp(const struct group* grp) {
    Group g;
    g.Gid = std::to_string(grp->gr_gid);
    g.Name = grp->gr_name;
    return g;
}

// 根据 GID 字符串查找组
inline Group LookupGroup(const std::string& gid) {
    gid_t gid_num = static_cast<gid_t>(std::stoul(gid));
    struct group grp;
    struct group* result = nullptr;
    long bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;
    std::vector<char> buffer(bufsize);

    int ret = getgrgid_r(gid_num, &grp, buffer.data(), bufsize, &result);
    if (ret != 0 || !result) {
        throw std::runtime_error("getgrgid_r failed: group not found");
    }
    return fillGroupFromGrp(&grp);
}

// 根据组名查找组
inline Group LookupGroupName(const std::string& name) {
    struct group grp;
    struct group* result = nullptr;
    long bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
    if (bufsize == -1) bufsize = 16384;
    std::vector<char> buffer(bufsize);

    int ret = getgrnam_r(name.c_str(), &grp, buffer.data(), bufsize, &result);
    if (ret != 0 || !result) {
        throw std::runtime_error("getgrnam_r failed: group not found");
    }
    return fillGroupFromGrp(&grp);
}

#endif  // _WIN32 / POSIX

}  // namespace bre::os::user
