#pragma once
/**
 * 处理 ip 相关事宜的工具类
 * ipv6 不支持 uint32_t 表示
 * 纯 C++ 实现，不依赖系统网络库
 */

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace bre {

using std::string;
using std::vector;

struct Ip {
public:
    Ip() = default;
    Ip(const std::string& ip_str);
    Ip(const std::vector<uint8_t>& ip_bytes);
    Ip(uint32_t ip_int);

    std::string ToString() const;
    std::vector<uint8_t> ToBytes() const;
    std::array<uint8_t, 16> ToBytesV6() const;
    std::array<uint8_t, 4> ToBytesV4() const;
    uint32_t ToInt() const;  // 仅 ipv4 有效

    bool IsIPv6() const { return is_ipv6; }
    bool IsIPv4() const { return !is_ipv6; }
    bool IsValid() const { return !ip_bytes.empty(); }
    bool IsLoopback() const { return IsLoopback(ip_bytes); }

    uint8_t operator[](size_t index) const;
    bool operator==(const Ip& other) const;
    bool operator!=(const Ip& other) const { return !(*this == other); }

    // ip 地址转换
    static vector<uint8_t> ToBytes(const string& ip_str);
    static vector<uint8_t> ToBytes(uint32_t ip_int);
    static string ToString(const vector<uint8_t>& ip_bytes);
    static string ToString(uint32_t ip_int);
    static uint32_t ToInt(const string& ip_str);
    static uint32_t ToInt(const vector<uint8_t>& ip_bytes);

    // ip 地址验证
    static bool IsValidIp(const string& ip_str);
    static bool IsValidIp(const vector<uint8_t>& ip_bytes);

    // 是否是合法的 组播 地址
    static bool IsMulticastIp(const string& ip_str);
    static bool IsMulticastIp(const vector<uint8_t>& ip_bytes);
    static bool IsMulticastIp(uint32_t ip_in);

    // 是否是合法的广播地址
    static bool IsBroadcastIp(const string& ip_str, const std::string& subnet_mask_str);
    static bool IsBroadcastIp(const vector<uint8_t>& ip_bytes,
                              const vector<uint8_t>& subnet_mask_bytes);
    static bool IsBroadcastIp(uint32_t ip_in, uint32_t subnet_mask_in);

    // 是否是回环地址
    static bool IsLoopback(const string& ip);
    static bool IsLoopback(const vector<uint8_t>& ip);
    static bool IsLoopback(uint32_t ip_int);

private:
    std::vector<uint8_t> ip_bytes;
    bool is_ipv6 = false;
};

// ==================== 构造函数实现 ====================

inline Ip::Ip(const std::string& ip_str) {
    ip_bytes = ToBytes(ip_str);
    is_ipv6 = (ip_bytes.size() == 16);
}

inline Ip::Ip(const std::vector<uint8_t>& bytes) {
    if (bytes.size() != 4 && bytes.size() != 16) {
        throw std::invalid_argument("IP bytes must be 4 (IPv4) or 16 (IPv6) bytes");
    }
    ip_bytes = bytes;
    is_ipv6 = (bytes.size() == 16);
}

inline Ip::Ip(uint32_t ip_int) {
    ip_bytes = ToBytes(ip_int);
    is_ipv6 = false;
}

// ==================== 成员方法实现 ====================
inline std::array<uint8_t, 16> Ip::ToBytesV6() const {
    if (!is_ipv6) {
        throw std::logic_error("Not an IPv6 address");
    }
    std::array<uint8_t, 16> arr{};
    std::copy(ip_bytes.begin(), ip_bytes.end(), arr.begin());
    return arr;
}

inline std::array<uint8_t, 4> Ip::ToBytesV4() const {
    if (is_ipv6) {
        throw std::logic_error("Not an IPv4 address");
    }
    std::array<uint8_t, 4> arr{};
    std::copy(ip_bytes.begin(), ip_bytes.end(), arr.begin());
    return arr;
}

inline std::string Ip::ToString() const { return ToString(ip_bytes); }

inline std::vector<uint8_t> Ip::ToBytes() const { return ip_bytes; }

inline uint32_t Ip::ToInt() const {
    if (is_ipv6) {
        throw std::logic_error("IPv6 cannot be converted to uint32_t");
    }
    return ToInt(ip_bytes);
}

inline uint8_t Ip::operator[](size_t index) const {
    if (index >= ip_bytes.size()) {
        throw std::out_of_range("IP address index out of range");
    }
    return ip_bytes[index];
}

inline bool Ip::operator==(const Ip& other) const {
    return is_ipv6 == other.is_ipv6 && ip_bytes == other.ip_bytes;
}

// ==================== 静态方法实现：地址转换 ====================

inline vector<uint8_t> Ip::ToBytes(const string& ip_str) {
    if (ip_str.empty()) return {};

    // 尝试解析 IPv4
    if (ip_str.find(':') == string::npos) {
        // IPv4 格式
        vector<uint8_t> result(4);
        std::istringstream iss(ip_str);
        int count = 0;
        int num;
        char dot;

        for (int i = 0; i < 4; ++i) {
            if (!(iss >> num)) return {};
            if (num < 0 || num > 255) return {};
            result[i] = static_cast<uint8_t>(num);
            count++;

            if (i < 3) {
                if (!(iss >> dot) || dot != '.') return {};
            }
        }

        // 确保没有多余的字符
        if (iss >> dot) return {};
        if (count == 4) return result;
        return {};
    }

    // IPv6 格式
    vector<uint8_t> result(16, 0);
    std::string_view sv(ip_str);

    // 查找 "::" 的位置
    size_t double_colon_pos = sv.find("::");
    bool has_double_colon = (double_colon_pos != std::string_view::npos);

    if (has_double_colon) {
        // 有 "::" 压缩形式
        std::string_view left = sv.substr(0, double_colon_pos);
        std::string_view right = (double_colon_pos + 2 < sv.length())
                                     ? sv.substr(double_colon_pos + 2)
                                     : std::string_view{};

        int left_count = 0;
        int right_count = 0;

        // 解析左边部分
        if (!left.empty()) {
            size_t start = 0;
            while (start < left.length()) {
                size_t end = left.find(':', start);
                if (end == std::string_view::npos) end = left.length();

                std::string_view group = left.substr(start, end - start);
                if (group.empty()) return {};

                uint16_t value = 0;
                for (char c : group) {
                    if (c >= '0' && c <= '9') {
                        value = value * 16 + (c - '0');
                    } else if (c >= 'a' && c <= 'f') {
                        value = value * 16 + (c - 'a' + 10);
                    } else if (c >= 'A' && c <= 'F') {
                        value = value * 16 + (c - 'A' + 10);
                    } else {
                        return {};
                    }
                    if (value > 0xFFFF) return {};
                }

                result[left_count * 2] = (value >> 8) & 0xFF;
                result[left_count * 2 + 1] = value & 0xFF;
                left_count++;

                start = end + 1;
            }
        }

        // 解析右边部分
        if (!right.empty()) {
            std::vector<uint16_t> right_groups;
            size_t start = 0;
            while (start < right.length()) {
                size_t end = right.find(':', start);
                if (end == std::string_view::npos) end = right.length();

                std::string_view group = right.substr(start, end - start);
                if (group.empty()) return {};

                uint16_t value = 0;
                for (char c : group) {
                    if (c >= '0' && c <= '9') {
                        value = value * 16 + (c - '0');
                    } else if (c >= 'a' && c <= 'f') {
                        value = value * 16 + (c - 'a' + 10);
                    } else if (c >= 'A' && c <= 'F') {
                        value = value * 16 + (c - 'A' + 10);
                    } else {
                        return {};
                    }
                    if (value > 0xFFFF) return {};
                }
                right_groups.push_back(value);
                start = end + 1;
            }

            right_count = right_groups.size();
            // 从后向前填充
            for (int i = 0; i < right_count; ++i) {
                int pos = 8 - right_count + i;
                result[pos * 2] = (right_groups[i] >> 8) & 0xFF;
                result[pos * 2 + 1] = right_groups[i] & 0xFF;
            }
        }

        if (left_count + right_count >= 8) return {};
    } else {
        // 完整格式，无压缩
        int group_count = 0;
        size_t start = 0;
        while (start < sv.length()) {
            size_t end = sv.find(':', start);
            if (end == std::string_view::npos) end = sv.length();

            std::string_view group = sv.substr(start, end - start);
            if (group.empty()) return {};

            uint16_t value = 0;
            for (char c : group) {
                if (c >= '0' && c <= '9') {
                    value = value * 16 + (c - '0');
                } else if (c >= 'a' && c <= 'f') {
                    value = value * 16 + (c - 'a' + 10);
                } else if (c >= 'A' && c <= 'F') {
                    value = value * 16 + (c - 'A' + 10);
                } else {
                    return {};
                }
                if (value > 0xFFFF) return {};
            }

            if (group_count >= 8) return {};
            result[group_count * 2] = (value >> 8) & 0xFF;
            result[group_count * 2 + 1] = value & 0xFF;
            group_count++;

            start = end + 1;
        }

        if (group_count != 8) return {};
    }

    return result;
}

inline vector<uint8_t> Ip::ToBytes(uint32_t ip_int) {
    vector<uint8_t> res(4);
    for (int i = 3; i >= 0; --i) {
        res[i] = ip_int & 0xFF;
        ip_int >>= 8;
    }
    return res;
}

inline string Ip::ToString(const vector<uint8_t>& ip_bytes) {
    if (ip_bytes.size() == 4) {
        // IPv4
        return std::to_string(ip_bytes[0]) + "." + std::to_string(ip_bytes[1]) + "." +
               std::to_string(ip_bytes[2]) + "." + std::to_string(ip_bytes[3]);
    }

    if (ip_bytes.size() == 16) {
        // IPv6
        std::ostringstream oss;

        // 找到最长的连续 0 段来压缩
        int max_zero_start = -1;
        int max_zero_len = 0;
        int current_zero_start = -1;
        int current_zero_len = 0;

        for (int i = 0; i < 8; ++i) {
            uint16_t value = (static_cast<uint16_t>(ip_bytes[i * 2]) << 8) | ip_bytes[i * 2 + 1];
            if (value == 0) {
                if (current_zero_start == -1) {
                    current_zero_start = i;
                    current_zero_len = 1;
                } else {
                    current_zero_len++;
                }
            } else {
                if (current_zero_len > max_zero_len && current_zero_len > 1) {
                    max_zero_start = current_zero_start;
                    max_zero_len = current_zero_len;
                }
                current_zero_start = -1;
                current_zero_len = 0;
            }
        }

        // 检查最后一段
        if (current_zero_len > max_zero_len && current_zero_len > 1) {
            max_zero_start = current_zero_start;
            max_zero_len = current_zero_len;
        }

        // 构建字符串
        for (int i = 0; i < 8; ++i) {
            if (max_zero_len > 0 && i == max_zero_start) {
                oss << "::";
                i += max_zero_len - 1;
            } else {
                uint16_t value =
                    (static_cast<uint16_t>(ip_bytes[i * 2]) << 8) | ip_bytes[i * 2 + 1];
                oss << std::hex << value;
                if (i < 7 && (max_zero_len == 0 || i + 1 != max_zero_start)) {
                    oss << ":";
                }
            }
        }

        return oss.str();
    }

    return {};
}

inline string Ip::ToString(uint32_t ip_int) { return ToString(ToBytes(ip_int)); }

inline uint32_t Ip::ToInt(const string& ip_str) {
    auto bytes = ToBytes(ip_str);
    if (bytes.size() != 4) {
        throw std::invalid_argument("Only IPv4 supported");
    }
    return ToInt(bytes);
}

inline uint32_t Ip::ToInt(const vector<uint8_t>& ip_bytes) {
    if (ip_bytes.size() != 4) throw std::invalid_argument("Only IPv4 supported");
    uint32_t val = 0;
    for (int i = 0; i < 4; ++i) val = (val << 8) | ip_bytes[i];
    return val;
}

// ip 地址验证 "192.168.100.123"
inline bool Ip::IsValidIp(const string& ip_str) {
    auto bytes = ToBytes(ip_str);
    return !bytes.empty() && (bytes.size() == 4 || bytes.size() == 16);
}

inline bool Ip::IsValidIp(const vector<uint8_t>& ip_bytes) {
    if (ip_bytes.size() != 4 && ip_bytes.size() != 16) return false;
    // 如果能转换成字符串再转换回来，说明是有效的
    auto str = ToString(ip_bytes);
    if (str.empty()) return false;
    auto bytes_back = ToBytes(str);
    return bytes_back == ip_bytes;
}


// 是否是合法的组播 地址
inline bool Ip::IsMulticastIp(const string& ip_str) {
    auto bytes = ToBytes(ip_str);
    if (bytes.empty()) return false;
    return IsMulticastIp(bytes);
}

inline bool Ip::IsMulticastIp(const vector<uint8_t>& ip_bytes) {
    if (ip_bytes.size() == 4) {
        return (ip_bytes[0] & 0xF0) == 0xE0;  // 224.0.0.0 ~ 239.255.255.255
    }
    if (ip_bytes.size() == 16) {
        return ip_bytes[0] == 0xFF;           // IPv6 组播
    }
    return false;
}

inline bool Ip::IsMulticastIp(uint32_t ip_in) {
    return (ip_in & 0xF0000000) == 0xE0000000;  // 224.0.0.0 ~ 239.255.255.255
}

inline bool Ip::IsBroadcastIp(const string& ip_str, const std::string& subnet_mask_str) {
    auto bytes = ToBytes(ip_str);
    auto subnet_mask_bytes = ToBytes(subnet_mask_str);
    if (bytes.empty() || subnet_mask_bytes.empty()) return false;
    return IsBroadcastIp(bytes, subnet_mask_bytes);
}

inline bool Ip::IsBroadcastIp(const vector<uint8_t>& ip_bytes,
                              const vector<uint8_t>& subnet_mask_bytes) {
    if (ip_bytes.size() != subnet_mask_bytes.size()) return false;
    if (ip_bytes.size() == 4) {
        uint32_t ip_int = ToInt(ip_bytes);
        uint32_t subnet_mask_int = ToInt(subnet_mask_bytes);
        return IsBroadcastIp(ip_int, subnet_mask_int);
    }
    if (ip_bytes.size() == 16) {
        // IPv6 broadcast address is not typically used, but if needed, implement here
        return false;
    }
    return false;
}

inline bool Ip::IsBroadcastIp(uint32_t ip_in, uint32_t subnet_mask_in) {
    return (ip_in | subnet_mask_in) == 0xFFFFFFFF;  // 255.255.255.255
}


// 是否是回环地址
inline bool Ip::IsLoopback(const string& ip) {
    auto bytes = ToBytes(ip);
    if (bytes.empty()) return false;
    return IsLoopback(bytes);
}

inline bool Ip::IsLoopback(const vector<uint8_t>& ip) {
    if (ip.size() == 4) {
        // 127.0.0.0/8
        return ip[0] == 127;
    }
    if (ip.size() == 16) {
        // ::1
        static const uint8_t loopback6[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};
        return std::memcmp(ip.data(), loopback6, 16) == 0;
    }
    return false;
}

inline bool Ip::IsLoopback(uint32_t ip_int) {
    // 127.0.0.0/8
    return ((ip_int >> 24) & 0xFF) == 127;
}


};  // namespace bre