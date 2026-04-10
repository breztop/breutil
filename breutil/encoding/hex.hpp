#pragma once


#include <bitset>
#include <cstdint>
#include <string>
#include <vector>

#include "../strconv.hpp"

namespace bre {

class Hex {
public:
    // to vector of bytes
    static std::vector<uint8_t> ToBytes(const std::string& hex);
    // from vector of bytes
    static std::string FromBytes(const std::vector<uint8_t>& data, bool uppercase = true);

    static std::string FormatInt(int, bool uppercase = true);
    static std::optional<int64_t> ParseInt(const std::string& hex);
};


inline std::vector<uint8_t> Hex::ToBytes(const std::string& hex) {
    std::vector<uint8_t> bytes;
    bytes.reserve(hex.size() / 2);
    for (size_t i = 0; i < hex.size(); i += 2) {
        uint8_t byte = (std::stoi(hex.substr(i, 2), nullptr, 16) & 0xFF);
        bytes.push_back(byte);
    }
    return bytes;
}


inline std::string Hex::FromBytes(const std::vector<uint8_t>& data, bool uppercase) {
    std::string hex;
    hex.reserve(data.size() * 2);
    const char* digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";
    for (uint8_t byte : data) {
        hex += digits[byte >> 4];
        hex += digits[byte & 0x0F];
    }
    return hex;
}

inline std::string Hex::FormatInt(int value, bool uppercase) {
    auto ret = strconv::FormatInt(value, 16);
    if (uppercase) {
        for (char& c : ret) {
            c = std::toupper(static_cast<unsigned char>(c));
        }
    }
    return ret;
}

inline std::optional<int64_t> Hex::ParseInt(const std::string& hex) {
    return strconv::ParseInt(hex, 16);
}


}  // namespace bre
