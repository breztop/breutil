#pragma once

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>

#include <string>


// 返回比如 15.3.2
inline std::string GetVersion() {
    CFStringRef versionKey = CFSTR("ProductVersion");
    CFURLRef url = CFURLCreateWithFileSystemPath(
        kCFAllocatorDefault, CFSTR("/System/Library/CoreServices/SystemVersion.plist"), kCFURLPOSIXPathStyle, false);
    CFDataRef resourceData = nullptr;
    SInt32 errorCode = 0;
    if (!CFURLCreateDataAndPropertiesFromResource(kCFAllocatorDefault, url, &resourceData, nullptr, nullptr,
                                                  &errorCode)) {
        if (url) CFRelease(url);
        return "";
    }
    CFPropertyListRef plist =
        CFPropertyListCreateWithData(kCFAllocatorDefault, resourceData, kCFPropertyListImmutable, nullptr, nullptr);
    std::string result;
    if (plist && CFGetTypeID(plist) == CFDictionaryGetTypeID()) {
        CFDictionaryRef dict = (CFDictionaryRef)plist;
        CFStringRef version = (CFStringRef)CFDictionaryGetValue(dict, versionKey);
        if (version) {
            char buffer[256];
            if (CFStringGetCString(version, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
                result = buffer;
            }
        }
    }
    if (plist) CFRelease(plist);
    if (resourceData) CFRelease(resourceData);
    if (url) CFRelease(url);
    return result;
}

#endif  // __APPLE__