#pragma once

#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace bre {

class FileWriter {
public:
    using Shared = std::shared_ptr<FileWriter>;

    Shared static Create(const std::string& Filename, bool is_write = false) {
        return std::make_shared<FileWriter>(Filename, is_write);
    }

    // 构造函数
    FileWriter(const std::string& Filename, bool is_write = false)
        : m_is_write(is_write)
        , filename(Filename) {
        if (!is_write) return;
        // 直接用成员变量打开文件
        outFile.open(filename, std::ios::binary);
        if (!outFile.is_open()) {
            std::cerr << "Failed to open file: " << filename << std::endl;
            throw std::runtime_error("Failed to open file: " + filename);
        }
    }


    // 写入数据到文件的方法
    bool WriteData(void* data, int size) {
        if (!m_is_write) {
            std::cout << "FileWriter is not write" << std::endl;
            return false;
        }

        if (data == nullptr || size <= 0) {
            return false;
        }

        // 写入数据到文件
        outFile.write(reinterpret_cast<char*>(data), size);

        // 检查是否成功写入
        if (!outFile.good()) {
            std::cerr << "Failed to write data to file: " << filename << std::endl;
            return false;
        }
        return true;
    }

    ~FileWriter() {
        if (m_is_write && outFile.is_open()) {
            outFile.flush();
            outFile.close();
        }
    }

private:
    bool m_is_write = false;
    // 使用二进制模式打开文件
    std::ofstream outFile;
    std::string filename;  // 文件名
};

}  // namespace bre
