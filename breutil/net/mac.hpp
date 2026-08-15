#pragma once

/** mac.hpp
 * 目前 仅支持 48 位 mac 地址， 用于网络 mac 地址的处理
 * mac字符串解析
 * 输入 -> mac字符串(“:”, “=”,“-”,“ ”,“.”, ""(无间隔))为间隔的6组16进制数
 */

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>

namespace bre {

class Mac {
public:
    Mac() : _mac{0, 0, 0, 0, 0, 0}, _valid(false) {}
    Mac(const std::string& mac_str);
    Mac(const std::array<uint8_t, 6>& bytes);

    std::string ToString(const std::string& spliter = ":") const;
    std::array<uint8_t, 6> ToBytes() const;
    bool IsValid() const;

    bool operator==(const Mac& other) const;
    bool operator!=(const Mac& other) const { return !(*this == other); }
    uint8_t operator[](size_t index) const;

    // 静态工具方法
    static std::string ToString(const std::array<uint8_t, 6>& bytes,
                                const std::string& spliter = ":");
    static bool IsValidMac(const std::string& mac_str);
    static std::array<uint8_t, 6> ToBytes(const std::string& mac_str);

private:
    std::array<uint8_t, 6> _mac;
    bool _valid;

    // 辅助函数：从十六进制字符转换为数字
    static int HexCharToInt(char c);
    // 辅助函数：解析 MAC 地址字符串
    static std::array<uint8_t, 6> ParseMacString(const std::string& mac_str, bool& valid);
};

// ==================== 辅助函数实现 ====================

inline int Mac::HexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

inline std::array<uint8_t, 6> Mac::ParseMacString(const std::string& mac_str, bool& valid) {
    std::array<uint8_t, 6> result = {0, 0, 0, 0, 0, 0};
    valid = false;

    if (mac_str.empty()) return result;

    // 移除所有分隔符，只保留十六进制字符
    std::string hex_only;
    for (char c : mac_str) {
        if (std::isxdigit(static_cast<unsigned char>(c))) {
            hex_only += c;
        } else if (c != ':' && c != '-' && c != '.' && c != ' ' && c != '=') {
            // 遇到非法字符
            return result;
        }
    }

    // MAC 地址应该是 12 个十六进制字符
    if (hex_only.size() != 12) return result;

    // 转换为字节数组
    for (size_t i = 0; i < 6; ++i) {
        int high = HexCharToInt(hex_only[i * 2]);
        int low = HexCharToInt(hex_only[i * 2 + 1]);
        if (high < 0 || low < 0) return result;
        result[i] = static_cast<uint8_t>((high << 4) | low);
    }

    valid = true;
    return result;
}

// ==================== 构造函数实现 ====================

inline Mac::Mac(const std::string& mac_str) { _mac = ParseMacString(mac_str, _valid); }

inline Mac::Mac(const std::array<uint8_t, 6>& bytes) : _mac(bytes), _valid(true) {}

// ==================== 成员方法实现 ====================

inline std::string Mac::ToString(const std::string& spliter) const {
    return Mac::ToString(_mac, spliter);
}

inline std::array<uint8_t, 6> Mac::ToBytes() const { return _mac; }

inline bool Mac::IsValid() const { return _valid; }

inline bool Mac::operator==(const Mac& other) const { return _mac == other._mac; }

inline uint8_t Mac::operator[](size_t index) const {
    if (index >= 6) {
        throw std::out_of_range("MAC address index out of range");
    }
    return _mac[index];
}

// ==================== 静态方法实现 ====================

inline std::string Mac::ToString(const std::array<uint8_t, 6>& bytes, const std::string& spliter) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0');

    for (size_t i = 0; i < 6; ++i) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
        if (i < 5 && !spliter.empty()) {
            oss << spliter;
        }
    }

    return oss.str();
}

inline bool Mac::IsValidMac(const std::string& mac_str) {
    bool valid = false;
    ParseMacString(mac_str, valid);
    return valid;
}

inline std::array<uint8_t, 6> Mac::ToBytes(const std::string& mac_str) {
    bool valid = false;
    auto result = ParseMacString(mac_str, valid);
    if (!valid) {
        throw std::invalid_argument("Invalid MAC address format");
    }
    return result;
}

};  // namespace bre
