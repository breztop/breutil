#pragma once
// 和 generate_build_info.cmake 配合使用，提供编译时的构建信息（如时间、Git状态等）
#include <string>

#ifdef BRE_BUILD_INFO_HEADER
#include BRE_BUILD_INFO_HEADER
#endif

namespace bre {

class GenerateBuildInfo {
    static constexpr int get_month(const char* date) {
        if (date[0] == 'J' && date[1] == 'a' && date[2] == 'n') return 1;
        if (date[0] == 'F' && date[1] == 'e' && date[2] == 'b') return 2;
        if (date[0] == 'M' && date[1] == 'a' && date[2] == 'r') return 3;
        if (date[0] == 'A' && date[1] == 'p' && date[2] == 'r') return 4;
        if (date[0] == 'M' && date[1] == 'a' && date[2] == 'y') return 5;
        if (date[0] == 'J' && date[1] == 'u' && date[2] == 'n') return 6;
        if (date[0] == 'J' && date[1] == 'u' && date[2] == 'l') return 7;
        if (date[0] == 'A' && date[1] == 'u' && date[2] == 'g') return 8;
        if (date[0] == 'S' && date[1] == 'e' && date[2] == 'p') return 9;
        if (date[0] == 'O' && date[1] == 'c' && date[2] == 't') return 10;
        if (date[0] == 'N' && date[1] == 'o' && date[2] == 'v') return 11;
        if (date[0] == 'D' && date[1] == 'e' && date[2] == 'c') return 12;
        return 0;
    }

public:
    static std::string GetBuildInfo() {
        std::string dirty_str = GetGitDirty() ? "dirty" : "clean";
        return "Built on: " + GetBuildTime() + " (" + GetBuildType() + ")\n" +
               "Git: " + GetGitBranch() + "@" + GetGitCommitHash() + " (" + dirty_str + ") [" +
               GetGitTag() + "]\n" + "Compiler: " + GetCompilerId() + " " + GetCompilerVersion() +
               "\n" + "Project Version: " + GetProjectVersion();
    }
#ifdef BRE_BUILD_INFO_HEADER
    static std::string GetBuildTime() { return BRE_BUILD_TIME; }
    static std::string GetGitCommitHash() { return BRE_GIT_COMMIT_HASH; }
    static std::string GetGitBranch() { return BRE_GIT_BRANCH; }
    static bool GetGitDirty() { return BRE_GIT_DIRTY; }
    static std::string GetGitTag() { return BRE_GIT_TAG; }
    static std::string GetBuildType() { return BRE_BUILD_TYPE; }
    static std::string GetCompilerId() { return BRE_COMPILER_ID; }
    static std::string GetCompilerVersion() { return BRE_COMPILER_VERSION; }
    static std::string GetProjectVersion() { return BRE_PROJECT_VERSION; }
#else


    static std::string GetBuildTime() {
        constexpr char time_str[] = __TIME__;
        constexpr int hour = (time_str[0] - '0') * 10 + (time_str[1] - '0');
        constexpr int min = (time_str[3] - '0') * 10 + (time_str[4] - '0');
        constexpr int sec = (time_str[6] - '0') * 10 + (time_str[7] - '0');

        // 2. 解析日期（纯 constexpr 月份匹配，无 strstr 依赖）
        constexpr char date_str[] = __DATE__;
        constexpr int month = get_month(date_str);
        // 处理日期可能的单字符（如 "Dec  5 2025" → 5，空格转0）
        constexpr int day1 = (date_str[4] == ' ') ? 0 : (date_str[4] - '0');
        constexpr int day2 = date_str[5] - '0';
        constexpr int day = day1 * 10 + day2;
        // 解析年份（__DATE__ 最后4位：如 2025 → date_str[7]~[10]）
        constexpr int year = (date_str[7] - '0') * 1000 + (date_str[8] - '0') * 100 +
                             (date_str[9] - '0') * 10 + (date_str[10] - '0');

        // 3. 格式化（自动补零，简洁无冗余）
        char buf[32] = {0};
        snprintf(buf, sizeof(buf), "%04d.%02d.%02d %02d:%02d:%02d", year, month, day, hour, min,
                 sec);
        return buf;
    }

    static std::string GetGitCommitHash() { return "Unknown"; }
    static std::string GetGitBranch() { return "Unknown"; }
    static bool GetGitDirty() { return false; }
    static std::string GetGitTag() { return "Unknown"; }
    static std::string GetBuildType() { return "Unknown"; }
    static std::string GetCompilerId() { return "Unknown"; }
    static std::string GetCompilerVersion() { return "Unknown"; }
    static std::string GetProjectVersion() { return "Unknown"; }
#endif  // #ifdef BRE_BUILD_INFO_HEADER
};

}  // namespace bre
