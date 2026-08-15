#pragma once

/**
 * NetworkInterface 网络接口类
 * 用于获取和管理本机网络接口信息（IP、MAC、接口名称等）
 * 如果网卡没有启动，不加入接口列表
 *
 * wifi 网络判断有待完善，一般会归在 OTHER / ETHERNET
 */

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef __linux__
#include <linux/if.h>
#include <linux/sockios.h>
#endif
#ifdef __APPLE__
#include <net/if_dl.h>
#endif
#endif

#include <map>
#include <string>
#include <vector>

#include "ip.hpp"
#include "mac.hpp"

namespace bre {

class NetworkInterface {
public:
    enum class InterfaceType { ANY, UNKNOWN, ETHERNET, WIFI, LOOPBACK, OTHER };
    enum class AddressType { ANY, IPV4, IPV6 };

    // 构造函数
    NetworkInterface() = default;
    NetworkInterface(const Ip& ip, const Mac& mac, const std::string& if_name_,
                     AddressType addr_type_, InterfaceType if_type_)
        : _ip(ip), _mac(mac), _if_name(if_name_), _addr_type(addr_type_), _if_type(if_type_) {}

    // 访问器
    const Ip& GetIp() const { return _ip; }
    const Mac& GetMac() const { return _mac; }
    const std::string& GetInterfaceName() const { return _if_name; }
    const std::string& GetMessage() const { return _message; }
    AddressType GetAddressType() const { return _addr_type; }
    InterfaceType GetInterfaceType() const { return _if_type; }

    void SetMessage(const std::string& msg) { _message = msg; }
    // 静态方法：获取网络接口列表
    static std::vector<NetworkInterface> Get(AddressType addr_type_filter = AddressType::ANY,
                                             InterfaceType if_type_filter = InterfaceType::ANY,
                                             bool refresh = false);

    // 静态方法：获取本机主机名
    static std::string GetHostName();

private:
    Ip _ip;                                                   // IP 地址
    Mac _mac;                                                 // MAC 地址
    std::string _if_name;                                     // 接口名称
    std::string _message;                                     // 其他信息
    AddressType _addr_type = AddressType::ANY;                // 地址类型
    InterfaceType _if_type = InterfaceType::UNKNOWN;          // 接口类型

    static std::vector<NetworkInterface> _cached_interfaces;  // 缓存的网络接口信息

    // 返回本机所有网络接口信息
    static void CollectInterfaceInfo();

    // 辅助函数：判断接口类型
    static InterfaceType DetermineInterfaceType(const std::string& if_name, bool is_loopback);
};

// ==================== 静态变量定义 ====================
inline std::vector<NetworkInterface> NetworkInterface::_cached_interfaces;

// ==================== 辅助函数实现 ====================

inline NetworkInterface::InterfaceType NetworkInterface::DetermineInterfaceType(
    const std::string& if_name, bool is_loopback) {
    if (is_loopback) return InterfaceType::LOOPBACK;

    // 根据接口名称判断类型
    if (if_name.find("eth") == 0 || if_name.find("en") == 0) {
        return InterfaceType::ETHERNET;
    } else if (if_name.find("wlan") == 0 || if_name.find("wifi") == 0 || if_name.find("wl") == 0) {
        return InterfaceType::WIFI;
    }

    return InterfaceType::OTHER;
}

// ==================== CollectInterfaceInfo 实现 ====================

inline void NetworkInterface::CollectInterfaceInfo() {
    _cached_interfaces.clear();

#ifdef _WIN32
    // Windows 实现
    IP_ADAPTER_ADDRESSES* adapter_addresses = nullptr;
    ULONG buffer_size = 15000;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;

    // 分配内存
    for (int attempts = 0; attempts < 3; ++attempts) {
        adapter_addresses = (IP_ADAPTER_ADDRESSES*)malloc(buffer_size);
        if (adapter_addresses == nullptr) return;

        ULONG result =
            GetAdaptersAddresses(AF_UNSPEC, flags, nullptr, adapter_addresses, &buffer_size);

        if (result == ERROR_SUCCESS) {
            break;
        } else {
            free(adapter_addresses);
            adapter_addresses = nullptr;
            if (result != ERROR_BUFFER_OVERFLOW) return;
        }
    }

    if (adapter_addresses == nullptr) return;

    // 遍历适配器
    for (PIP_ADAPTER_ADDRESSES adapter = adapter_addresses; adapter != nullptr;
         adapter = adapter->Next) {
        // 跳过非活动接口
        if (adapter->OperStatus != IfOperStatusUp) continue;

        std::string if_name = "";
        // 将 FriendlyName 从宽字符转换为窄字符
        if (adapter->FriendlyName) {
            int size = WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, nullptr, 0,
                                           nullptr, nullptr);
            if (size > 0) {
                std::vector<char> buffer(size);
                WideCharToMultiByte(CP_UTF8, 0, adapter->FriendlyName, -1, buffer.data(), size,
                                    nullptr, nullptr);
                if_name = buffer.data();
            }
        }

        // 获取 MAC 地址
        Mac mac_addr;
        if (adapter->PhysicalAddressLength == 6) {
            std::array<uint8_t, 6> mac_bytes;
            for (int i = 0; i < 6; ++i) {
                mac_bytes[i] = adapter->PhysicalAddress[i];
            }
            mac_addr = Mac(mac_bytes);
        }

        // 判断接口类型
        bool is_loopback = (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK);
        InterfaceType if_type = DetermineInterfaceType(if_name, is_loopback);

        // 遍历 IP 地址
        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress; unicast != nullptr;
             unicast = unicast->Next) {
            sockaddr* sa = unicast->Address.lpSockaddr;

            std::string ip_str;
            AddressType addr_type = AddressType::ANY;

            if (sa->sa_family == AF_INET) {
                char buf[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &((struct sockaddr_in*)sa)->sin_addr, buf, sizeof(buf));
                ip_str = buf;
                addr_type = AddressType::IPV4;
            } else if (sa->sa_family == AF_INET6) {
                char buf[INET6_ADDRSTRLEN];
                inet_ntop(AF_INET6, &((struct sockaddr_in6*)sa)->sin6_addr, buf, sizeof(buf));
                ip_str = buf;
                addr_type = AddressType::IPV6;
            }

            if (!ip_str.empty()) {
                Ip ip_addr(ip_str);
                _cached_interfaces.emplace_back(ip_addr, mac_addr, if_name, addr_type, if_type);
            }
        }
    }

    free(adapter_addresses);

#else
    // Unix/Linux/macOS 实现
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) != 0) return;

#if defined(__APPLE__)
    // macOS: 先收集所有接口的 MAC 地址
    std::map<std::string, Mac> ifname2mac;
    for (auto p = ifaddr; p != nullptr; p = p->ifa_next) {
        if (p->ifa_addr && p->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)p->ifa_addr;
            if (sdl->sdl_alen == 6) {
                uint8_t* mac = (uint8_t*)LLADDR(sdl);
                std::array<uint8_t, 6> mac_bytes;
                for (int i = 0; i < 6; ++i) {
                    mac_bytes[i] = mac[i];
                }
                ifname2mac[p->ifa_name] = Mac(mac_bytes);
            }
        }
    }
#endif

    // 遍历所有网络接口
    for (auto p = ifaddr; p != nullptr; p = p->ifa_next) {
        if (!p->ifa_addr) continue;

        // 只处理 IP 地址
        if (p->ifa_addr->sa_family != AF_INET && p->ifa_addr->sa_family != AF_INET6) continue;

        std::string if_name = p->ifa_name ? p->ifa_name : "";
        bool is_up = (p->ifa_flags & IFF_UP);
        bool is_loopback = (p->ifa_flags & IFF_LOOPBACK);

        // 跳过未启动的接口
        if (!is_up) continue;

        // 获取 IP 地址
        std::string ip_str;
        AddressType addr_type = AddressType::ANY;

        if (p->ifa_addr->sa_family == AF_INET) {
            char buf[INET_ADDRSTRLEN];
            auto sa = (struct sockaddr_in*)p->ifa_addr;
            inet_ntop(AF_INET, &sa->sin_addr, buf, sizeof(buf));
            ip_str = buf;
            addr_type = AddressType::IPV4;
        } else if (p->ifa_addr->sa_family == AF_INET6) {
            char buf[INET6_ADDRSTRLEN];
            auto sa = (struct sockaddr_in6*)p->ifa_addr;
            inet_ntop(AF_INET6, &sa->sin6_addr, buf, sizeof(buf));
            ip_str = buf;
            addr_type = AddressType::IPV6;
        }

        if (ip_str.empty()) continue;

        // 获取 MAC 地址
        Mac mac_addr;

#if defined(__linux__)
        // Linux: 使用 ioctl 获取 MAC 地址
        int fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (fd >= 0) {
            struct ifreq ifr;
            std::strncpy(ifr.ifr_name, p->ifa_name, IFNAMSIZ - 1);
            ifr.ifr_name[IFNAMSIZ - 1] = '\0';
            if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
                std::array<uint8_t, 6> mac_bytes;
                for (int i = 0; i < 6; ++i) {
                    mac_bytes[i] = (uint8_t)ifr.ifr_hwaddr.sa_data[i];
                }
                mac_addr = Mac(mac_bytes);
            }
            close(fd);
        }
#elif defined(__APPLE__)
        // macOS: 从之前收集的 MAC 地址中查找
        auto it = ifname2mac.find(if_name);
        if (it != ifname2mac.end()) {
            mac_addr = it->second;
        }
#endif

        // 判断接口类型
        InterfaceType if_type = DetermineInterfaceType(if_name, is_loopback);

        // 创建 NetworkInterface 对象并添加到列表
        Ip ip_addr(ip_str);
        _cached_interfaces.emplace_back(ip_addr, mac_addr, if_name, addr_type, if_type);
    }

    freeifaddrs(ifaddr);
#endif
}

// ==================== Get 方法实现 ====================

inline std::vector<NetworkInterface> NetworkInterface::Get(AddressType addr_type_filter,
                                                           InterfaceType if_type_filter,
                                                           bool refresh) {
    // 如果需要刷新或者缓存为空，重新收集信息
    if (refresh || _cached_interfaces.empty()) {
        CollectInterfaceInfo();
    }

    // 如果不需要过滤，直接返回所有接口
    if (addr_type_filter == AddressType::ANY && if_type_filter == InterfaceType::ANY) {
        return _cached_interfaces;
    }

    // 根据过滤条件筛选
    std::vector<NetworkInterface> result;
    for (const auto& iface : _cached_interfaces) {
        bool addr_match =
            (addr_type_filter == AddressType::ANY || iface._addr_type == addr_type_filter);
        bool if_match =
            (if_type_filter == InterfaceType::ANY || iface._if_type == if_type_filter);

        if (addr_match && if_match) {
            result.push_back(iface);
        }
    }

    return result;
}

// ==================== GetHostName 实现 ====================

inline std::string NetworkInterface::GetHostName() {
    char buf[256] = {0};
    if (gethostname(buf, sizeof(buf)) == 0) {
        return std::string(buf);
    }
    return {};
}

}  // namespace bre
