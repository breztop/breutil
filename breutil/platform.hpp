#pragma once

#ifdef __CYGWIN__
#define BRE_OS_CYGWIN 1
#elif defined(__MINGW__) || defined(__MINGW32__) || defined(__MINGW64__)
#define BRE_OS_WINDOWS_MINGW 1
#define BRE_OS_WINDOWS 1
#elif defined _WIN32
#define BRE_OS_WINDOWS 1
#ifdef _WIN32_WCE
#define BRE_OS_WINDOWS_MOBILE 1
#elif defined(WINAPI_FAMILY)
#include <winapifamily.h>
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
#define BRE_OS_WINDOWS_DESKTOP 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE_APP)
#define BRE_OS_WINDOWS_PHONE 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)
#define BRE_OS_WINDOWS_RT 1
#define BRE_OS_WINDOWS_UWP 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_TV_TITLE)
#define BRE_OS_WINDOWS_PHONE 1
#define BRE_OS_WINDOWS_TV_TITLE 1
#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_GAMES)
#define BRE_OS_WINDOWS_GAMES 1
#else
// WINAPI_FAMILY defined but no known partition matched.
// Default to desktop.
#define BRE_OS_WINDOWS_DESKTOP 1
#endif
#else
#define BRE_OS_WINDOWS_DESKTOP 1
#endif  // _WIN32_WCE
#elif defined __OS2__
#define BRE_OS_OS2 1
#elif defined __APPLE__
#define BRE_OS_MAC 1
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define BRE_OS_IOS 1
#endif
#if TARGET_OS_VISION
#define BRE_OS_VISION 1
#endif
#elif defined __DragonFly__
#define BRE_OS_DRAGONFLY 1
#elif defined __FreeBSD__
#define BRE_OS_FREEBSD 1
#elif defined __Fuchsia__
#define BRE_OS_FUCHSIA 1
#elif defined(__GNU__)
#define BRE_OS_GNU_HURD 1
#elif defined(__GLIBC__) && defined(__FreeBSD_kernel__)
#define BRE_OS_GNU_KFREEBSD 1
#elif defined __linux__
#define BRE_OS_LINUX 1
#if defined __ANDROID__
#define BRE_OS_LINUX_ANDROID 1
#endif
#elif defined __MVS__
#define BRE_OS_ZOS 1
#elif defined(__sun) && defined(__SVR4)
#define BRE_OS_SOLARIS 1
#elif defined(_AIX)
#define BRE_OS_AIX 1
#elif defined(__hpux)
#define BRE_OS_HPUX 1
#elif defined __native_client__
#define BRE_OS_NACL 1
#elif defined __NetBSD__
#define BRE_OS_NETBSD 1
#elif defined __OpenBSD__
#define BRE_OS_OPENBSD 1
#elif defined __QNX__
#define BRE_OS_QNX 1
#elif defined(__HAIKU__)
#define BRE_OS_HAIKU 1
#elif defined ESP8266
#define BRE_OS_ESP8266 1
#elif defined ESP32
#define BRE_OS_ESP32 1
#elif defined(__XTENSA__)
#define BRE_OS_XTENSA 1
#elif defined(__hexagon__)
#define BRE_OS_QURT 1
#elif defined(CPU_QN9090) || defined(CPU_QN9090HN)
#define BRE_OS_NXP_QN9090 1
#elif defined(NRF52)
#define BRE_OS_NRF52 1
#endif  // __CYGWIN__

// Define BRE_OS_POSIX for POSIX-compliant systems
#if defined(BRE_OS_LINUX) || defined(BRE_OS_MAC) || defined(BRE_OS_IOS) ||          \
    defined(BRE_OS_FREEBSD) || defined(BRE_OS_OPENBSD) || defined(BRE_OS_NETBSD) || \
    defined(BRE_OS_DRAGONFLY) || defined(BRE_OS_SOLARIS) || defined(BRE_OS_AIX) ||  \
    defined(BRE_OS_HPUX) || defined(BRE_OS_QNX) || defined(BRE_OS_GNU_HURD) ||      \
    defined(BRE_OS_GNU_KFREEBSD) || defined(BRE_OS_HAIKU)
#define BRE_OS_POSIX 1
#endif


#if !defined(BRE_OS_WINDOWS) && !defined(BRE_OS_LINUX) && !defined(BRE_OS_MAC) && \
    !defined(BRE_OS_IOS) && !defined(BRE_OS_LINUX_ANDROID)
#error "Do not support current target system!"
#endif
