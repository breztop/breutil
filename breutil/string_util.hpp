#pragma once
#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <string>
#include <vector>

/*
1. 分割与连接
Split(const std::string&, char delimiter)
根据单个字符分隔符分割字符串，返回vector<string>。
Split(const std::string&, const std::string& delimiter)
根据字符串分隔符分割字符串，返回vector<string>。
Join(const std::vector<std::string>&, const std::string& separator)
用分隔符连接字符串列表，返回拼接后的字符串。

2. 查找与替换
FindAll(const std::string&, const std::string&)
返回所有匹配子串的起始位置的vector<size_t>。
ReplaceAll(std::string&, const std::string&, const std::string&)
替换字符串中所有匹配子串。
ReplaceFirst(std::string&, const std::string&, const std::string&)
替换字符串中第一个匹配子串。

3. 大小写转换
ToUpper(std::string&)
将字符串转为大写（修改原字符串）。
ToLower(std::string&)
将字符串转为小写（修改原字符串）。
ToUpperCopy(const std::string&)
返回大写的新字符串（不修改原字符串）。
ToLowerCopy(const std::string&)
返回小写的新字符串（不修改原字符串）。

4. 检查函数
StartsWith(const std::string&, const std::string&)
检查字符串是否以指定前缀开头。
EndsWith(const std::string&, const std::string&)
检查字符串是否以指定后缀结尾。
Contains(const std::string&, const std::string&)
检查字符串是否包含指定子串。

5. Trim函数
Trim(std::string&)
去除字符串前后空格（修改原字符串）。
LTrim(std::string&)
去除字符串左端空格（修改原字符串）。
RTrim(std::string&)
去除字符串右端空格（修改原字符串）。
TrimCopy(const std::string&)
返回去除前后空格的新字符串（不修改原字符串）。
6. 转换函数

ToInt(const std::string&)
将字符串转为整数（抛出异常或返回错误码）。
ToDouble(const std::string&)
将字符串转为双精度浮点数。

7. 其他实用函数
LPad(const std::string&, size_t width, char pad_char)
左填充字符串至指定长度。
RPad(const std::string&, size_t width, char pad_char)
右填充字符串至指定长度。
Reverse(const std::string&)
返回反转后的字符串。
*/
namespace bre {
namespace str {

inline static std::vector<std::string> Split(const std::string& str, char delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
    }
    result.push_back(str.substr(start));
    return result;
}


inline static std::vector<std::string> Split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0, end = 0;
    while ((end = str.find(delimiter, start)) != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
    }
    result.push_back(str.substr(start));
    return result;
}


inline static std::string Join(const std::vector<std::string>& strings,
                               const std::string& separator) {
    if (strings.empty()) return "";
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
        result += separator + strings[i];
    }
    return result;
}

inline static std::vector<size_t> FindAll(const std::string& str, const std::string& sub) {
    std::vector<size_t> positions;
    size_t pos = str.find(sub);
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = str.find(sub, pos + sub.length());
    }
    return positions;
}

inline static void ReplaceAll(std::string& str, const std::string& old_sub,
                              const std::string& new_sub) {
    size_t pos = 0;
    while ((pos = str.find(old_sub, pos)) != std::string::npos) {
        str.replace(pos, old_sub.length(), new_sub);
        pos += new_sub.length();
    }
}

inline static void ReplaceFirst(std::string& str, const std::string& old_sub,
                                const std::string& new_sub) {
    size_t pos = str.find(old_sub);
    if (pos != std::string::npos) {
        str.replace(pos, old_sub.length(), new_sub);
    }
}

inline static std::string ToUpperCopy(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

inline static std::string ToLowerCopy(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

inline static void ToUpper(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

inline static void ToLower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

inline static bool StartsWith(const std::string& str, const std::string& prefix) {
    return str.rfind(prefix, 0) == 0;
}

inline static bool EndsWith(const std::string& str, const std::string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

inline static bool Contains(const std::string& str, const std::string& sub) {
    return str.find(sub) != std::string::npos;
}

inline static std::string TrimCopy(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos) return "";  // 全是空格
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}

inline static std::string LTrimCopy(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    return (first == std::string::npos) ? "" : str.substr(first);
}

inline static std::string RTrimCopy(const std::string& str) {
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return (last == std::string::npos) ? "" : str.substr(0, last + 1);
}

inline static void LTrim(std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first != std::string::npos) {
        str.erase(0, first);
    } else {
        str.clear();  // 全是空格
    }
}

inline static void RTrim(std::string& str) {
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    if (last != std::string::npos) {
        str.erase(last + 1);
    } else {
        str.clear();  // 全是空格
    }
}

inline static void Trim(std::string& str) {
    LTrim(str);
    RTrim(str);
}


inline static int ToInt(const std::string& str, int base = 10) {
    int value;
    auto [ptr, ec] = std::from_chars(str.data(), str.data() + str.size(), value, base);
    if (ec != std::errc()) {
        throw std::invalid_argument("Invalid integer string: " + str);
    }
    return value;
}

inline static double ToDouble(const std::string& str) {
    double value;

    value = std::stod(str);
    std::errc ec = std::errc();
    if (ec != std::errc()) {
        throw std::invalid_argument("Invalid double string: " + str);
    }
    return value;
}

inline static void LPad(std::string& str, size_t width, char pad_char = ' ') {
    if (str.length() < width) {
        str.insert(str.begin(), width - str.length(), pad_char);
    }
}

inline static void RPad(std::string& str, size_t width, char pad_char = ' ') {
    if (str.length() < width) {
        str.append(width - str.length(), pad_char);
    }
}

inline static void Reverse(std::string& str) { std::reverse(str.begin(), str.end()); }

}  // namespace str

}  // namespace bre
