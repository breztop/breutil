#pragma once

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>


namespace bre {
namespace strconv {

inline bool IsPrint(char32_t r) {
    if (r < 0x20u || r == 0x7Fu) {
        return false;
    }
    // For ASCII we can determine directly, for non-ASCII assume printable if not control.
    if (r < 0x80u) {
        return true;
    }
    if (r >= 0xD800u && r <= 0xDFFFu) {  // surrogate pair range
        return false;
    }
    return true;
}

inline bool IsGraphic(char32_t r) {
    if (!IsPrint(r)) {
        return false;
    }
    if (r == U' ' || r == U'\t' || r == U'\n' || r == U'\r' || r == U'\f' || r == U'\v') {
        return false;
    }
    return true;
}


inline std::optional<int> Atoi(const std::string& s) {
    if (s.empty()) {
        return std::nullopt;
    }

    const char* start = s.data();
    const char* end = s.data() + s.size();

    if (*start == '+') {
        ++start;
        if (start == end) {
            return std::nullopt;
        }
        if (*start == '-' || *start == '+') {
            return std::nullopt;
        }
    }


    int result;
    auto [ptr, ec] = std::from_chars(start, end, result);

    if (ec == std::errc{} && ptr == end) {
        return result;
    }

    return std::nullopt;
}

inline std::optional<bool> ParseBool(const std::string& s) {
    std::string lower = s;
    for (auto& c : lower) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

    if (lower == "true" || lower == "1" || lower == "t" || lower == "yes") {
        return true;
    }
    if (lower == "false" || lower == "0" || lower == "f" || lower == "no") {
        return false;
    }
    return std::nullopt;
}

inline std::string FormatBool(bool b) { return b ? "true" : "false"; }

/**
 * 将整数转换为字符串，支持指定进制（2-36）。默认使用 10 进制。
 * @param value 要转换的整数值
 * @param base 进制，范围为 2 到 36。默认值为 10。
 * @note 对于负数，结果字符串会以 '-' 开头。对于 16
 * 进制，字母部分使用小写（a-f）。对于无效的进制值，返回空字符串。
 */
inline std::string FormatInt(int64_t value, int base = 10) {
    if (base < 2 || base > 36) {
        return "";
    }
    char buf[65];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, base);
    if (ec != std::errc{}) {
        return "";
    }
    return std::string(buf, ptr);
}

inline std::string FormatUint(uint64_t value, int base = 10) {
    if (base < 2 || base > 36) {
        return "";
    }
    char buf[65];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), value, base);
    if (ec != std::errc{}) {
        return "";
    }
    return std::string(buf, ptr);
}

inline std::string Itoa(int i) { return FormatInt(i, 10); }

inline std::optional<int64_t> ParseInt(const std::string& s, int base = 10, int bitSize = 0) {
    if (s.empty()) return std::nullopt;

    int usedBase = base;
    size_t start = 0;
    bool negative = false;
    if (s[0] == '+' || s[0] == '-') {
        start = 1;
        negative = (s[0] == '-');
    }

    std::string_view sv(s.c_str() + start, s.size() - start);
    if (usedBase == 0) {
        if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'x' || sv[1] == 'X')) {
            usedBase = 16;
            sv.remove_prefix(2);
        } else if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'b' || sv[1] == 'B')) {
            usedBase = 2;
            sv.remove_prefix(2);
        } else if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'o' || sv[1] == 'O')) {
            usedBase = 8;
            sv.remove_prefix(2);
        } else if (sv.size() > 0 && sv[0] == '0') {
            usedBase = 8;
            sv.remove_prefix(1);
        } else {
            usedBase = 10;
        }
    }
    if (usedBase < 2 || usedBase > 36) return std::nullopt;

    if (sv.empty()) return std::nullopt;

    int64_t value = 0;
    const char* p = sv.data();
    const char* end = sv.data() + sv.size();
    while (p < end) {
        char c = *p;
        int digit;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'z')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            digit = c - 'A' + 10;
        else
            return std::nullopt;
        if (digit >= usedBase) return std::nullopt;

        uint64_t next = static_cast<uint64_t>(value) * usedBase + digit;
        if (negative) {
            if (next > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1ULL)
                return std::nullopt;
        } else {
            if (next > static_cast<uint64_t>(std::numeric_limits<int64_t>::max()))
                return std::nullopt;
        }
        value = static_cast<int64_t>(next);
        ++p;
    }

    if (negative) {
        value = -value;
    }

    if (bitSize == 0 || bitSize == 64) {
        return value;
    }
    if (bitSize == 32) {
        if (value < std::numeric_limits<int32_t>::min() ||
            value > std::numeric_limits<int32_t>::max())
            return std::nullopt;
    }
    return value;
}

inline std::optional<uint64_t> ParseUint(const std::string& s, int base = 10, int bitSize = 0) {
    if (s.empty()) {
        return std::nullopt;
    }

    size_t start = 0;
    if (s[0] == '+' || s[0] == '-') {
        if (s[0] == '-') {
            return std::nullopt;
        }
        start = 1;
    }

    std::string_view sv(s.c_str() + start, s.size() - start);
    int usedBase = base;
    if (usedBase == 0) {
        if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'x' || sv[1] == 'X')) {
            usedBase = 16;
            sv.remove_prefix(2);
        } else if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'b' || sv[1] == 'B')) {
            usedBase = 2;
            sv.remove_prefix(2);
        } else if (sv.size() > 1 && sv[0] == '0' && (sv[1] == 'o' || sv[1] == 'O')) {
            usedBase = 8;
            sv.remove_prefix(2);
        } else if (sv.size() > 0 && sv[0] == '0') {
            usedBase = 8;
            sv.remove_prefix(1);
        } else {
            usedBase = 10;
        }
    }
    if (usedBase < 2 || usedBase > 36) return std::nullopt;
    if (sv.empty()) return std::nullopt;

    uint64_t value = 0;
    for (char c : sv) {
        int digit;
        if (c >= '0' && c <= '9')
            digit = c - '0';
        else if (c >= 'a' && c <= 'z')
            digit = c - 'a' + 10;
        else if (c >= 'A' && c <= 'Z')
            digit = c - 'A' + 10;
        else
            return std::nullopt;
        if (digit >= usedBase) return std::nullopt;
        if (value > (std::numeric_limits<uint64_t>::max() - digit) / usedBase) return std::nullopt;
        value = value * usedBase + digit;
    }

    if (bitSize == 0 || bitSize == 64) {
        return value;
    }
    if (bitSize == 32 && value > std::numeric_limits<uint32_t>::max()) {
        return std::nullopt;
    }

    return value;
}

inline std::optional<double> ParseDouble(const std::string& s) {
    if (s.empty()) {
        return std::nullopt;
    }

    const char* start = s.data();
    const char* end = s.data() + s.size();

    if (*start == '+') {
        ++start;

        if (start == end) {
            return std::nullopt;
        }

        if (*start == '-' || *start == '+') {
            return std::nullopt;
        }
    }

#if defined(__APPLE__)
    if (*start == ' ') {
        return std::nullopt;
    }

    std::string temp(start, end);
    char* parseEnd = nullptr;
    errno = 0;

    double val = std::strtod(temp.c_str(), &parseEnd);
    if (errno != 0 || parseEnd != temp.c_str() + temp.size()) {
        return std::nullopt;
    }

    return val;
#else
    double val;
    auto [ptr, ec] = std::from_chars(start, end, val);

    if (ec != std::errc{} || ptr != end) {
        return std::nullopt;
    }

    return val;
#endif
}

inline std::string FormatDouble(double f, char fmt = 'g', int prec = -1) {
    std::ostringstream oss;
    if (prec >= 0) {
        oss << std::setprecision(prec);
    }
    switch (fmt) {
        case 'f':
        case 'F':
            oss << std::fixed;
            break;
        case 'e':
        case 'E':
            oss << std::scientific;
            break;
        default:
            break;
    }
    oss << f;
    return oss.str();
}


inline std::string QuoteChar32(char32_t r) {
    std::string out;
    if (r == '\\')
        out = "\\\\";
    else if (r == '\"')
        out = "\\\"";
    else if (r == '\'')
        out = "\\'";
    else if (r == '\a')
        out = "\\a";
    else if (r == '\b')
        out = "\\b";
    else if (r == '\f')
        out = "\\f";
    else if (r == '\n')
        out = "\\n";
    else if (r == '\r')
        out = "\\r";
    else if (r == '\t')
        out = "\\t";
    else if (r == '\v')
        out = "\\v";
    else if (r < 0x20u || r == 0x7Fu) {
        char buf[10];
        std::snprintf(buf, sizeof(buf), "\\x%02x", static_cast<unsigned>(r));
        out = buf;
    } else {
        if (r < 0x80u)
            out = std::string(1, static_cast<char>(r));
        else {
            // Unicode fallback using UTF-8
            if (r <= 0x7FFu) {
                out.push_back(static_cast<char>(0xC0 | ((r >> 6) & 0x1F)));
                out.push_back(static_cast<char>(0x80 | (r & 0x3F)));
            } else if (r <= 0xFFFFu) {
                out.push_back(static_cast<char>(0xE0 | ((r >> 12) & 0x0F)));
                out.push_back(static_cast<char>(0x80 | ((r >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (r & 0x3F)));
            } else {
                out.push_back(static_cast<char>(0xF0 | ((r >> 18) & 0x07)));
                out.push_back(static_cast<char>(0x80 | ((r >> 12) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | ((r >> 6) & 0x3F)));
                out.push_back(static_cast<char>(0x80 | (r & 0x3F)));
            }
        }
    }
    return "'" + out + "'";
}

inline std::string Quote(const std::string& s) {
    std::ostringstream oss;
    oss << '"';
    for (unsigned char c : s) {
        switch (c) {
            case '\\':
                oss << "\\\\";
                break;
            case '\"':
                oss << "\\\"";
                break;
            case '\a':
                oss << "\\a";
                break;
            case '\b':
                oss << "\\b";
                break;
            case '\f':
                oss << "\\f";
                break;
            case '\n':
                oss << "\\n";
                break;
            case '\r':
                oss << "\\r";
                break;
            case '\t':
                oss << "\\t";
                break;
            case '\v':
                oss << "\\v";
                break;
            default:
                if (c < 0x20 || c == 0x7f) {
                    char buf[5];
                    std::snprintf(buf, sizeof(buf), "\\x%02x", c);
                    oss << buf;
                } else {
                    oss << c;
                }
        }
    }
    oss << '"';
    return oss.str();
}

inline std::string QuoteToASCII(const std::string& s) {
    std::ostringstream oss;
    oss << '"';
    for (unsigned char c : s) {
        if (c < 0x20 || c == 0x7f || c >= 0x80) {
            char buf[5];
            std::snprintf(buf, sizeof(buf), "\\x%02x", c);
            oss << buf;
        } else if (c == '\\') {
            oss << "\\\\";
        } else if (c == '"') {
            oss << "\\\"";
        } else {
            oss << c;
        }
    }
    oss << '"';
    return oss.str();
}

inline std::string QuoteToGraphic(const std::string& s) {
    std::ostringstream oss;
    oss << '"';
    for (unsigned char c : s) {
        if (!IsGraphic(static_cast<char32_t>(c))) {
            char buf[5];
            std::snprintf(buf, sizeof(buf), "\\x%02x", c);
            oss << buf;
        } else if (c == '\\') {
            oss << "\\\\";
        } else if (c == '"') {
            oss << "\\\"";
        } else {
            oss << c;
        }
    }
    oss << '"';
    return oss.str();
}

inline std::pair<std::string, std::optional<std::string>> QuotedPrefix(const std::string& s) {
    if (s.empty()) return {"", std::nullopt};
    char q = s[0];
    if (q == '"' || q == '\'') {
        size_t pos = s.find_first_of(q, 1);
        if (pos == std::string::npos)
            return {"", std::make_optional<std::string>("missing end quote")};
        std::string payload = s.substr(0, pos + 1);
        return {payload, std::nullopt};
    }
    return {"", std::make_optional<std::string>("invalid quote")};
}

inline std::optional<std::string> Unquote(const std::string& s) {
    if (s.size() < 2) return std::nullopt;
    char q = s.front();
    if (s.back() != q) return std::nullopt;
    if (q == '`') {
        return s.substr(1, s.size() - 2);
    }

    std::string out;
    for (size_t i = 1; i + 1 < s.size(); ++i) {
        char c = s[i];
        if (c != '\\') {
            out.push_back(c);
            continue;
        }
        if (i + 1 >= s.size() - 1) return std::nullopt;
        char nxt = s[++i];
        switch (nxt) {
            case 'a':
                out.push_back('\a');
                break;
            case 'b':
                out.push_back('\b');
                break;
            case 'f':
                out.push_back('\f');
                break;
            case 'n':
                out.push_back('\n');
                break;
            case 'r':
                out.push_back('\r');
                break;
            case 't':
                out.push_back('\t');
                break;
            case 'v':
                out.push_back('\v');
                break;
            case '\\':
                out.push_back('\\');
                break;
            case '\"':
                out.push_back('"');
                break;
            case '\'':
                out.push_back('\'');
                break;
            case 'x': {
                if (i + 2 >= s.size() - 1) return std::nullopt;
                unsigned d;
                if (std::sscanf(s.c_str() + i + 1, "%2x", &d) != 1) return std::nullopt;
                out.push_back(static_cast<char>(d));
                i += 2;
                break;
            }
            default:
                out.push_back(nxt);
                break;
        }
    }
    return out;
}

inline std::tuple<char32_t, bool, std::string, std::optional<std::string>> UnquoteChar(
    const std::string& s, char quote) {
    if (s.empty()) return {0, false, "", std::make_optional<std::string>("empty string")};
    if (s[0] == quote) {
        return {0, false, s, std::make_optional<std::string>("first char is quote")};
    }

    if (s[0] != '\\') {
        return {static_cast<char32_t>(s[0]), false, s.substr(1), std::nullopt};
    }
    if (s.size() < 2) return {0, false, "", std::make_optional<std::string>("incomplete escape")};
    char next = s[1];
    switch (next) {
        case 'a':
            return {'\a', false, s.substr(2), std::nullopt};
        case 'b':
            return {'\b', false, s.substr(2), std::nullopt};
        case 'f':
            return {'\f', false, s.substr(2), std::nullopt};
        case 'n':
            return {'\n', false, s.substr(2), std::nullopt};
        case 'r':
            return {'\r', false, s.substr(2), std::nullopt};
        case 't':
            return {'\t', false, s.substr(2), std::nullopt};
        case 'v':
            return {'\v', false, s.substr(2), std::nullopt};
        case '\\':
            return {'\\', false, s.substr(2), std::nullopt};
        case '"':
            return {'"', false, s.substr(2), std::nullopt};
        case '\'':
            return {'\'', false, s.substr(2), std::nullopt};
        case 'x': {
            if (s.size() < 4)
                return {0, false, "", std::make_optional<std::string>("incomplete hex escape")};
            unsigned d;
            if (std::sscanf(s.c_str() + 2, "%2x", &d) != 1)
                return {0, false, "", std::make_optional<std::string>("bad hex escape")};
            return {static_cast<char32_t>(d), false, s.substr(4), std::nullopt};
        }
        default:
            return {static_cast<char32_t>(next), false, s.substr(2), std::nullopt};
    }
}

}  // namespace strconv
}  // namespace bre
