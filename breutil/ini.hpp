#pragma once
#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <vector>

/*
第一次调用 Ini::Instance()时，会加载配置文件，之后的调用都会返回同一个实例

使用
```cpp
bre::Ini& config = bre::Ini::Instance("config.ini")["section"];
std::string value = config.GetValue("key", "default");
int intValue = config.GetInt("int_key", 42);
```

支持的INI文件格式特性：
- 支持 ; 和 # 两种注释符
- 支持引号字符串（单引号和双引号）
- 支持转义字符 (\n, \t, \\, \", \')
- 支持多种数据类型获取（string, int, double, bool）
- 支持配置修改和保存
- 线程安全

# todo:
- 增加对数组类型的支持（例如 key=value1,value2,value3）
- 增加对include其他INI文件的支持
- 增加对环境变量的支持（例如 key=${ENV_VAR}）
*/

namespace bre {

struct IniSection {
    IniSection() {}
    ~IniSection() { _section_datas.clear(); }

    IniSection(const IniSection& src) { _section_datas = src._section_datas; }

    IniSection& operator=(const IniSection& src) {
        if (this == &src) {
            return *this;
        }

        this->_section_datas = src._section_datas;
        return *this;
    }

    std::map<std::string, std::string> _section_datas;

    std::string operator[](const std::string& key) {
        if (_section_datas.find(key) == _section_datas.end()) {
            return "";
        }
        return _section_datas[key];
    }

    std::string GetStr(const std::string& key, const std::string& defaultValue = "") {
        if (_section_datas.find(key) == _section_datas.end()) {
            return defaultValue;
        }
        return _section_datas[key];
    }

    int GetInt(const std::string& key, int defaultValue = 0) {
        auto it = _section_datas.find(key);
        if (it == _section_datas.end() || it->second.empty()) {
            return defaultValue;
        }
        try {
            return std::stoi(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }

    double GetDouble(const std::string& key, double defaultValue = 0.0) {
        auto it = _section_datas.find(key);
        if (it == _section_datas.end() || it->second.empty()) {
            return defaultValue;
        }
        try {
            return std::stod(it->second);
        } catch (const std::exception&) {
            return defaultValue;
        }
    }

    bool GetBool(const std::string& key, bool defaultValue = false) {
        auto it = _section_datas.find(key);
        if (it == _section_datas.end() || it->second.empty()) {
            return defaultValue;
        }
        std::string value = it->second;
        // 转换为小写
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);
        if (value == "true" || value == "1" || value == "yes" || value == "on") {
            return true;
        } else if (value == "false" || value == "0" || value == "no" || value == "off") {
            return false;
        }
        return defaultValue;
    }

    bool HasKey(const std::string& key) const {
        return _section_datas.find(key) != _section_datas.end();
    }

    void SetValue(const std::string& key, const std::string& value) { _section_datas[key] = value; }

    std::vector<std::string> GetAllKeys() const {
        std::vector<std::string> keys;
        for (const auto& pair : _section_datas) {
            keys.push_back(pair.first);
        }
        return keys;
    }
};

class Ini {
public:
    ~Ini() = default;
    Ini(const Ini& src) = delete;
    Ini(Ini&& src) noexcept
        : _config_map(std::move(src._config_map))
        , _file_path(std::move(src._file_path)) {}
    Ini& operator=(const Ini& src) {
        if (this == &src) {
            return *this;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        _file_path = src._file_path;
        _config_map = src._config_map;
        return *this;
    }
    Ini& operator=(Ini&& src) noexcept {
        if (this == &src) {
            return *this;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        _file_path = std::move(src._file_path);
        _config_map = std::move(src._config_map);
        return *this;
    }

    IniSection& operator[](const std::string& section) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            _config_map[section] = IniSection();
        }
        return _config_map[section];
    }

    // 获取单例实例，null_ini参数用于创建空配置实例，用于保存默认配置
    static Ini& Instance(const std::string& filePath, bool null_ini = false) {
        if (filePath.empty()) {
            std::string err_msg = "[Ini] Instance called with empty filePath";
            std::cerr << err_msg << std::endl;
            throw(std::runtime_error(err_msg));
        }

        static std::map<std::string, Ini> instances;
        static std::mutex inst_mutex;
        std::lock_guard<std::mutex> lock(inst_mutex);
        auto it = instances.find(filePath);
        if (it == instances.end()) {
            auto result = instances.emplace(filePath, Ini{filePath});
            it = result.first;
            if (!null_ini) {
                it->second.load(filePath);
            }
        }
        return it->second;
    }


    // 重新加载配置文件
    bool Reload(const std::string& filePath) { return load(filePath); }

    // 获取字符串值
    std::string GetStr(const std::string& section, const std::string& key,
                       const std::string& defaultValue = "") {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return defaultValue;
        }
        return _config_map[section].GetStr(key, defaultValue);
    }

    // 获取整数值
    int GetInt(const std::string& section, const std::string& key, int defaultValue = 0) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return defaultValue;
        }
        return _config_map[section].GetInt(key, defaultValue);
    }

    // 获取浮点数值
    double GetDouble(const std::string& section, const std::string& key,
                     double defaultValue = 0.0) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return defaultValue;
        }
        return _config_map[section].GetDouble(key, defaultValue);
    }

    // 获取布尔值
    bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return defaultValue;
        }
        return _config_map[section].GetBool(key, defaultValue);
    }

    // 设置值
    void Set(const std::string& section, const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _config_map[section].SetValue(key, value);
    }

    // 设置整数值
    void Set(const std::string& section, const std::string& key, int value) {
        Set(section, key, std::to_string(value));
    }

    // 设置浮点数值
    void Set(const std::string& section, const std::string& key, double value) {
        Set(section, key, std::to_string(value));
    }

    // 设置布尔值（使用模板确保只接受真正的bool类型）
    template <typename T>
        requires std::is_same_v<T, bool>
    void Set(const std::string& section, const std::string& key, T value) {
        std::string val_str = value ? "true" : "false";
        Set(section, key, val_str);
    }

    // 检查section是否存在
    bool HasSection(const std::string& section) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _config_map.find(section) != _config_map.end();
    }

    // 检查key是否存在
    bool HasKey(const std::string& section, const std::string& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return false;
        }
        return _config_map[section].HasKey(key);
    }

    // 获取所有section
    std::vector<std::string> GetAllSections() {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> sections;
        for (const auto& pair : _config_map) {
            sections.push_back(pair.first);
        }
        return sections;
    }

    // 获取某个section下的所有key
    std::vector<std::string> GetAllKeys(const std::string& section) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return std::vector<std::string>();
        }
        return _config_map[section].GetAllKeys();
    }

    // 保存到文件
    bool Save() {
        std::lock_guard<std::mutex> lock(_mutex);
        std::string path = _file_path;
        if (path.empty()) {
            std::cerr << "[Ini] No file path specified for saving" << std::endl;
            return false;
        }

        std::ofstream file(path);
        if (!file.is_open()) {
            std::cerr << "[Ini] Failed to open file for writing: " << path << std::endl;
            return false;
        }

        for (const auto& section_pair : _config_map) {
            file << "[" << section_pair.first << "]" << std::endl;
            for (const auto& kv_pair : section_pair.second._section_datas) {
                file << kv_pair.first << "=" << kv_pair.second << std::endl;
            }
            file << std::endl;
        }

        file.close();
        return true;
    }

    // 删除section
    bool RemoveSection(const std::string& section) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _config_map.erase(section) > 0;
    }

    // 删除某个key
    bool RemoveKey(const std::string& section, const std::string& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_config_map.find(section) == _config_map.end()) {
            return false;
        }
        return _config_map[section]._section_datas.erase(key) > 0;
    }

private:
    Ini(std::string file_path) : _file_path(file_path) {}

    // 去除字符串首尾空白
    static std::string trim(const std::string& str) {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos) {
            return "";
        }
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, last - first + 1);
    }

    // 处理转义字符
    static std::string unescapeString(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        bool escaped = false;

        for (char c : str) {
            if (escaped) {
                switch (c) {
                    case 'n':
                        result += '\n';
                        break;
                    case 't':
                        result += '\t';
                        break;
                    case 'r':
                        result += '\r';
                        break;
                    case '\\':
                        result += '\\';
                        break;
                    case '"':
                        result += '"';
                        break;
                    case '\'':
                        result += '\'';
                        break;
                    default:
                        result += c;
                        break;
                }
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else {
                result += c;
            }
        }
        return result;
    }

    // 处理引号字符串
    static std::string processQuotedString(std::string str) {
        str = trim(str);
        if (str.empty()) {
            return str;
        }

        // 检查是否是引号包裹的字符串
        if ((str.front() == '"' && str.back() == '"') ||
            (str.front() == '\'' && str.back() == '\'')) {
            str = str.substr(1, str.size() - 2);
            return unescapeString(str);
        }
        return str;
    }


    bool load(const std::string& filename) {
        std::lock_guard<std::mutex> lock(_mutex);
        _file_path = filename;

        std::ifstream file(filename);

        _config_map.clear();

        if (!file.is_open()) {
            std::string error_msg = "[Ini] Failed to open file: " + filename;
            std::cerr << error_msg << std::endl;
            throw(std::runtime_error(error_msg));
            return false;
        }

        std::string line;
        std::string current_section;
        int line_number = 0;

        while (std::getline(file, line)) {
            line_number++;

            // 移除注释 (支持 ; 和 #)
            size_t comment_pos = line.find_first_of(";#");
            if (comment_pos != std::string::npos) {
                line = line.substr(0, comment_pos);
            }

            // 去除首尾空白
            line = trim(line);

            if (line.empty()) {
                continue;
            }

            // 处理section
            if (line.front() == '[' && line.back() == ']') {
                current_section = line.substr(1, line.size() - 2);
                current_section = trim(current_section);
                if (current_section.empty()) {
                    std::cerr << "[Ini] Warning: Empty section name at line " << line_number
                              << std::endl;
                }
            } else {
                // 处理键值对
                size_t equal_pos = line.find('=');
                if (equal_pos != std::string::npos) {
                    std::string key = line.substr(0, equal_pos);
                    std::string value = line.substr(equal_pos + 1);

                    key = trim(key);
                    value = processQuotedString(value);

                    if (key.empty()) {
                        std::cerr << "[Ini] Warning: Empty key at line " << line_number
                                  << std::endl;
                        continue;
                    }

                    if (current_section.empty()) {
                        std::cerr << "[Ini] Warning: Key-value pair outside of section at line "
                                  << line_number << std::endl;
                        continue;
                    }

                    _config_map[current_section]._section_datas[key] = value;
                } else {
                    std::cerr << "[Ini] Warning: Invalid line format at line " << line_number
                              << ": " << line << std::endl;
                }
            }
        }

        file.close();
        return true;
    }

    std::map<std::string, IniSection> _config_map;
    std::string _file_path;
    std::mutex _mutex;
};

}  // namespace bre
