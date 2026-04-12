
#ifdef _MSC_VER
#define BRE_WARN_PUSH _Pragma("warning(push)")
#define BRE_WARN_POP _Pragma("warning(pop)")
#define BRE_WARN_DEPRECATION _Pragma("warning(disable: 4996)")
#elif defined(__clang__)
#define BRE_WARN_PUSH _Pragma("clang diagnostic push")
#define BRE_WARN_POP _Pragma("clang diagnostic pop")
#define BRE_WARN_DEPRECATION _Pragma("clang diagnostic warning \"-Wdeprecated-declarations\"")
#elif defined(__GNUC__)
#define BRE_WARN_PUSH _Pragma("GCC diagnostic push")
#define BRE_WARN_POP _Pragma("GCC diagnostic pop")
#define BRE_WARN_DEPRECATION _Pragma("GCC diagnostic warning \"-Wdeprecated-declarations\"")
#else
#define BRE_WARN_PUSH
#define BRE_WARN_POP
#define BRE_WARN_DEPRECATION
#endif