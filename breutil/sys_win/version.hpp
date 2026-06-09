#pragma once
// 原代码：WebRTC 的代码

#include <Windows.h>
#include <stddef.h>

#include <string>


namespace bre {


enum class version_alias {
    VERSION_PRE_XP = 0,  // Not supported.
    VERSION_XP = 1,
    VERSION_SERVER_2003 = 2,   // Also includes XP Pro x64 and Server 2003 R2.
    VERSION_VISTA = 3,         // Also includes Windows Server 2008.
    VERSION_WIN7 = 4,          // Also includes Windows Server 2008 R2.
    VERSION_WIN8 = 5,          // Also includes Windows Server 2012.
    VERSION_WIN8_1 = 6,        // Also includes Windows Server 2012 R2.
    VERSION_WIN10 = 7,         // Threshold 1: Version 1507, Build 10240.
    VERSION_WIN10_TH2 = 8,     // Threshold 2: Version 1511, Build 10586.
    VERSION_WIN10_RS1 = 9,     // Redstone 1: Version 1607, Build 14393.
    VERSION_WIN10_RS2 = 10,    // Redstone 2: Version 1703, Build 15063.
    VERSION_WIN10_RS3 = 11,    // Redstone 3: Version 1709, Build 16299.
    VERSION_WIN10_RS4 = 12,    // Redstone 4: Version 1803, Build 17134.
    VERSION_WIN10_RS5 = 13,    // Redstone 5: Version 1809, Build 17763.
    VERSION_WIN10_19H1 = 14,   // 19H1: Version 1903, Build 18362.
    VERSION_WIN10_19H2 = 15,   // 19H2: Version 1909, Build 18363.
    VERSION_WIN10_20H1 = 16,   // 20H1: Version 2004, Build 19041.
    VERSION_WIN10_20H2 = 17,   // 20H2: Build 19042.
    VERSION_WIN10_21H1 = 18,   // 21H1: Build 19043.
    VERSION_WIN10_21H2 = 19,   // 21H2: Build 19044.
    VERSION_SERVER_2022 = 20,  // Server 2022: Build 20348.
    VERSION_WIN11 = 21,        // Windows 11: Build 22000.
    VERSION_WIN_LAST,          // Indicates error condition.
} version_alias;

enum version_type {
    SUITE_HOME = 0,
    SUITE_PROFESSIONAL,
    SUITE_SERVER,
    SUITE_ENTERPRISE,
    SUITE_EDUCATION,
    SUITE_LAST,
};

class os_info {
public:
    struct version_number {
        int major;
        int minor;
        int build;
        int patch;
    };
    struct service_pack {
        int major;
        int minor;
    };
    enum win_architecture {
        X86_ARCHITECTURE,
        X64_ARCHITECTURE,
        IA64_ARCHITECTURE,
        OTHER_ARCHITECTURE,
    };

    std::string processor_model_name();

private:
    version_alias version_;
    version_number number_;
    version_type type_;
    service_pack pack_;

    std::string service_pack_str_;
    win_architecture architecture_;
    int processors_;
    size_t allocation_granularity_;

    std::string processor_model_name_;
};

// Because this is by far the most commonly-requested value from the above
// singleton, we add a global-scope accessor here as syntactic sugar.
version_alias os_get_version();

#include <memory>

#if !defined(__clang__) && _MSC_FULL_VER < 191125507
#error VS 2017 Update 3.2 or higher is required
#endif

#if !defined(WINUWP)

namespace {

version_alias major_minor_build_to_version(int major, int minor, int build) {
    using enum version_alias;
    if ((major == 5) && (minor > 0)) {
        // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
        return (minor == 1) ? VERSION_XP : VERSION_SERVER_2003;
    } else if (major == 6) {
        switch (minor) {
            case 0:
                // Treat Windows Server 2008 the same as Windows Vista.
                return VERSION_VISTA;
            case 1:
                // Treat Windows Server 2008 R2 the same as Windows 7.
                return VERSION_WIN7;
            case 2:
                // Treat Windows Server 2012 the same as Windows 8.
                return VERSION_WIN8;
            default:
                return VERSION_WIN8_1;
        }
    } else if (major == 10) {
        if (build < 10586) {
            return VERSION_WIN10;
        } else if (build < 14393) {
            return VERSION_WIN10_TH2;
        } else if (build < 15063) {
            return VERSION_WIN10_RS1;
        } else if (build < 16299) {
            return VERSION_WIN10_RS2;
        } else if (build < 17134) {
            return VERSION_WIN10_RS3;
        } else if (build < 17763) {
            return VERSION_WIN10_RS4;
        } else if (build < 18362) {
            return VERSION_WIN10_RS5;
        } else if (build < 18363) {
            return VERSION_WIN10_19H1;
        } else if (build < 19041) {
            return VERSION_WIN10_19H2;
        } else if (build < 19042) {
            return VERSION_WIN10_20H1;
        } else if (build < 19043) {
            return VERSION_WIN10_20H2;
        } else if (build < 19044) {
            return VERSION_WIN10_21H1;
        } else if (build < 20348) {
            return VERSION_WIN10_21H2;
        } else if (build < 22000) {
            return VERSION_SERVER_2022;
        } else {
            return VERSION_WIN11;
        }
    } else if (major == 11) {
        return VERSION_WIN11;
    } else if (major > 6) {
        return VERSION_WIN_LAST;
    }

    return VERSION_PRE_XP;
}

int get_ubr() {
#if defined(WINUWP)
    return 0;
#else
    static constexpr wchar_t k_reg_key_windows_nt_current_version[] =
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";

    registry key;
    if (key.open(HKEY_LOCAL_MACHINE, k_reg_key_windows_nt_current_version, KEY_QUERY_VALUE) != ERROR_SUCCESS) {
        return 0;
    }

    DWORD ubr = 0;
    key.read(L"UBR", &ubr);

    return static_cast<int>(ubr);
#endif
}

bool internal_get_version_numbers(OSVERSIONINFOW *osvi) {
    if (osvi == nullptr) return false;

    HINSTANCE hins = LoadLibraryA("ntdll.dll");
    if (!hins) {
#pragma warning(push)
#pragma warning(disable : 4996)
        return GetVersionExW(osvi);
#pragma warning(pop)
    }
    auto ntproc = (void(__stdcall *)(DWORD *, DWORD *, DWORD *))GetProcAddress(hins, "RtlGetNtVersionNumbers");
    if (ntproc) {
        ntproc(&osvi->dwMajorVersion, &osvi->dwMinorVersion, &osvi->dwBuildNumber);
        osvi->dwBuildNumber &= 0xFFFF osvi->dwPlatformId = VER_PLATFORM_WIN32_NT;
    }
    FreeLibrary(hins);
    return ntproc != nullptr;
}
}  // namespace
#endif
os_info::os_info() : version_(VERSION_PRE_XP), architecture_(OTHER_ARCHITECTURE) {
    OSVERSIONINFOEXW version_info = {sizeof version_info};
    internal_get_version_numbers(reinterpret_cast<OSVERSIONINFOW *>(&version_info));
    number_.major = version_info.dwMajorVersion;
    number_.minor = version_info.dwMinorVersion;
    number_.build = version_info.dwBuildNumber;
    number_.patch = get_ubr();
    version_ = major_minor_build_to_version(number_.major, number_.minor, number_.build);
    pack_.major = version_info.wServicePackMajor;
    pack_.minor = version_info.wServicePackMinor;
    service_pack_str_ = version_info.szCSDVersion;
    SYSTEM_INFO system_info = {};
    ::GetNativeSystemInfo(&system_info);
    switch (system_info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_INTEL:
            architecture_ = X86_ARCHITECTURE;
            break;
        case PROCESSOR_ARCHITECTURE_AMD64:
            architecture_ = X64_ARCHITECTURE;
            break;
        case PROCESSOR_ARCHITECTURE_IA64:
            architecture_ = IA64_ARCHITECTURE;
            break;
    }
    processors_ = system_info.dwNumberOfProcessors;
    allocation_granularity_ = system_info.dwAllocationGranularity;

#if !defined(WINUWP)
    DWORD os_type;

    if (version_info.dwMajorVersion == 6 || version_info.dwMajorVersion == 10) {
        auto get_product_info = reinterpret_cast<BOOL(WINAPI *)(DWORD, DWORD, DWORD, DWORD, PDWORD)>(
            ::GetProcAddress(::GetModuleHandleW(L"kernel32.dll"), "GetProductInfo"));
        using enum version_alias;
        get_product_info(version_info.dwMajorVersion, version_info.dwMinorVersion, 0, 0, &os_type);
        switch (os_type) {
            case PRODUCT_STANDARD_SERVER:
                type_ = SUITE_SERVER;
                break;
            case PRODUCT_ULTIMATE:
                type_ = SUITE_PROFESSIONAL;
                break;
            case PRODUCT_BUSINESS:
                type_ = SUITE_ENTERPRISE;
                break;
            case PRODUCT_EDUCATION:
                type_ = SUITE_EDUCATION;
                break;
            case PRODUCT_HOME_BASIC:

            default:
                type_ = SUITE_HOME;
                break;
        }
    } else if (version_info.dwMajorVersion == 5 && version_info.dwMinorVersion == 2) {
        if (version_info.wProductType == VER_NT_WORKSTATION &&
            system_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
            type_ = SUITE_PROFESSIONAL;
        } else if (version_info.wSuiteMask & VER_SUITE_WH_SERVER) {
            type_ = SUITE_HOME;
        } else {
            type_ = SUITE_SERVER;
        }
    } else if (version_info.dwMajorVersion == 5 && version_info.dwMinorVersion == 1) {
        if (version_info.wSuiteMask & VER_SUITE_PERSONAL)
            type_ = SUITE_HOME;
        else
            type_ = SUITE_PROFESSIONAL;
    } else {
        // Windows is pre XP so we don't care but pick a safe default.
        type_ = SUITE_HOME;
    }
#else
    type_ = SUITE_HOME;
#endif  // !defined(WINUWP)
}

os_info::~os_info() {}

std::string os_info::processor_model_name() {
#if defined(WINUWP)
    return "Unknown Processor (UWP)";
#else
    if (processor_model_name_.empty()) {
        const wchar_t k_processor_name_string[] = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0";
        registry key(HKEY_LOCAL_MACHINE, k_processor_name_string, KEY_READ);
        std::wstring value;
        key.read(L"ProcessorNameString", value);
        processor_model_name_ = value;
    }
    return processor_model_name_;
#endif  // defined(WINUWP)
}


}  // namespace bre
