#pragma once

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <vector>

namespace bre {

#pragma pack(push, 1)
struct NetHeader {
    // 4 字节  BREZ 标识符
    // 4 字节 文件大小
    // 4 字节 组ID
    // 20字节 保留
    static const size_t HeaderSize = 32;

    std::vector<uint8_t> Data;
    uint32_t FileSize;

    NetHeader() : Data(HeaderSize) {
        Data[0] = 'B';
        Data[1] = 'R';
        Data[2] = 'E';
        Data[3] = 'Z';
        this->FileSize = 0;
    }

    bool Parse(const std::vector<uint8_t>& data) {
        if (data.size() < HeaderSize) {
            std::cout << "Header size is too small: " << data.size() << std::endl;
            for (auto i : data) {
                std::cout << i << " ";
            }
            std::cerr << "Header size is too small\n";
            return false;
        }
        if (data[0] != 'B' || data[1] != 'R' || data[2] != 'E' || data[3] != 'Z') {
            std::cerr << "Header is not BREZ\n";
            return false;
        }
        FileSize = *reinterpret_cast<const uint32_t*>(data.data() + 4);
        Data = data;
        return true;
    }

    size_t GetHeaderSize() { return Data.size(); }

    std::vector<uint8_t> GetHeaderData(uint64_t size) {
        FileSize = static_cast<uint32_t>(size);
        std::copy_n(reinterpret_cast<const uint8_t*>(&FileSize), sizeof(FileSize), Data.data() + 4);
        return Data;
    }

    void Reset() {
        Data[0] = 'B';
        Data[1] = 'R';
        Data[2] = 'E';
        Data[3] = 'Z';
        FileSize = 0;
    }
};
#pragma pack(pop)

}  // namespace bre
