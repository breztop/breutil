#ifdef __APPLE__
#pragma once

#include <CoreFoundation/CoreFoundation.h>

#include <string>

namespace bre {

inline std::string ConvertCFStringToStdString(CFStringRef cfStr) {
    if (!cfStr) {
        return "";
    }
    std::string str;
    // 获得长度
    CFIndex length = CFStringGetLength(cfStr);
    if (length == 0) {
        return str;
    }
    // str 最后一个 ‘\0’
    length++;
    char* buffer = new char[length];
    memset(buffer, 0, length);
    if (CFStringGetCString(cfStr, buffer, length, kCFStringEncodingUTF8)) {
        str = buffer;
    }
    delete[] buffer;
    return str;
}

}  // namespace bre
#endif  // __APPLE__