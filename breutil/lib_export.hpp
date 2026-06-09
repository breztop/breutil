#pragma once

#if defined(_WIN32)
    #ifdef BRE_IMPORT_DLL
        #define BRE_EXPORT __declspec(dllexport)
    #else
        #define BRE_EXPORT __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define BRE_EXPORT __attribute__((visibility("default")))
#else
    #define BRE_EXPORT
#endif
