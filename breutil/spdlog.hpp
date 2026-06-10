#pragma once
// 如果使用spdlog，关闭bre::Log
#ifdef USE_SPDLOG
#include <spdlog/sinks/ansicolor_sink.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#define LOG(LEVEL, ...)                                                            \
    spdlog::default_logger()->log(spdlog::source_loc(__FILE__, __LINE__, nullptr), \
                                  static_cast<spdlog::level::level_enum>(LEVEL), __VA_ARGS__)

#define LOG_TRACE(...) LOG(spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(...) LOG(spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG(spdlog::level::err, __VA_ARGS__)
#define LOG_FATAL(...) LOG(spdlog::level::critical, __VA_ARGS__)
#else
#include <filesystem>
#include <format>
#include <iostream>
#include <mutex>
#include <string>


namespace bre {

constexpr const char* COLOR_DARKRED = "\033[1;30m";
constexpr const char* COLOR_RED = "\033[1;31m";
constexpr const char* COLOR_YELLOW = "\033[1;33m";
constexpr const char* COLOR_GREEN = "\033[1;32m";
constexpr const char* COLOR_BLUE = "\033[1;34m";
constexpr const char* COLOR_RESET = "\033[0m";


enum class LogLevel { trace = 0, debug, info, warn, error, fatal };

static LogLevel LOG_LEVEL = LogLevel::debug;  // 0: debug, 1: info, 2: warn, 3: error, 4: fatal


template <typename... Args>
static void log(LogLevel level, const char* file, int line, std::format_string<Args...> fmt,
                Args&&... args) {
    // static std::mutex log_mutex;
    static auto& log_mutex = *new std::mutex;
    if (level < LOG_LEVEL) return;

    const char* level_str = "";
    const char* color = COLOR_RESET;
    switch (level) {
        case LogLevel::trace:
            level_str = "TRACE";
            color = COLOR_DARKRED;
            break;
        case LogLevel::debug:
            level_str = "DEBUG";
            color = COLOR_YELLOW;
            break;
        case LogLevel::info:
            level_str = "INFO";
            color = COLOR_GREEN;
            break;
        case LogLevel::warn:
            level_str = "WARN";
            color = COLOR_BLUE;
            break;
        case LogLevel::error:
            level_str = "ERROR";
            color = COLOR_RED;
            break;
        case LogLevel::fatal:
            level_str = "FATAL";
            color = COLOR_DARKRED;
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }

    std::lock_guard<std::mutex> lock(log_mutex);

    std::string message = std::format(fmt, std::forward<Args>(args)...);
    std::string filename = std::filesystem::path(file).filename().string();

    std::cout << color << "[" << level_str << "] " << filename << ":" << line << ": " << COLOR_RESET
              << message << std::endl;
    std::cout.flush();
}


}  // namespace bre

#define LOG(LEVEL, ...) bre::log(LEVEL, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_TRACE(...) LOG(bre::LogLevel::trace, __VA_ARGS__)
#define LOG_DEBUG(...) LOG(bre::LogLevel::debug, __VA_ARGS__)
#define LOG_INFO(...) LOG(bre::LogLevel::info, __VA_ARGS__)
#define LOG_WARN(...) LOG(bre::LogLevel::warn, __VA_ARGS__)
#define LOG_ERROR(...) LOG(bre::LogLevel::error, __VA_ARGS__)
#define LOG_FATAL(...) LOG(bre::LogLevel::fatal, __VA_ARGS__)

inline void set_log_level(bre::LogLevel level) { bre::LOG_LEVEL = level; }

#endif  // USE_SPDLOG
