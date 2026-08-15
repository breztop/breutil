#pragma once

#include <cctype>
#include <charconv>
#include <string>
#include <string_view>

#include "json_exception.hpp"
#include "json_value.hpp"

namespace bre {
namespace json {
class Parser {
public:
    static Value parse(const std::string& str, bool is_strict = false) {
        if (is_strict) {
            Parser parser(str);
            return parser.parseValue();
        } else {
            std::string cleanStr = removeComments(removeTailComma(str));
            Parser parser(std::move(cleanStr));
            return parser.parseValue();
        }
    }

    static void parse(const std::string& str, Value& root, bool is_strict = false) {
        if (is_strict) {
            Parser parser(str);
            root = parser.parseValue();
        } else {
            std::string cleanStr = removeComments(removeTailComma(str));
            Parser parser(std::move(cleanStr));
            root = parser.parseValue();
        }
    }

private:
    explicit Parser(const std::string& str) : input_(str), position_(0) {}

    Value parseValue() {
        skipWhitespace();
        if (position_ >= input_.size()) {
            throwException("Unexpected end of input");
        }

        switch (input_[position_]) {
            case 'n':
                return parseNull();
            case 't':
            case 'f':
                return parseBool();
            case '"':
                return parseString();
            case '[':
                return parseArray();
            case '{':
                return parseObject();
            default:
                return parseDefault();
        }
    }

    // 解析数字
    Value parseDefault() {
        bool ret = std::isdigit(input_[position_]) || input_[position_] == '-';

        if (!ret) {
            throwException("Unexpected character");
        }

        return parseNumber();
    }

    Value parseNull() {
        expect("null");
        return Value();
    }

    Value parseBool() {
        if (input_.substr(position_, 4) == std::string_view("true")) {
            position_ += 4;
            return Value(true);
        } else if (input_.substr(position_, 5) == std::string_view("false")) {
            position_ += 5;
            return Value(false);
        } else {
            throwException("Invalid boolean value");
        }
    }

    Value parseNumber() {
        const char* start = input_.data() + position_;
        const char* p = start;
        const char* end = input_.data() + input_.size();

        // Handle sign
        if (p < end && (*p == '-' || *p == '+')) ++p;

        // Count digits before '.'
        const char* intPartStart = p;
        while (p < end && std::isdigit(*p)) ++p;
        bool hasIntDigits = (p != intPartStart);

        bool isFloat = false;

        // Check for '.'
        if (p < end && *p == '.') {
            isFloat = true;
            ++p;
            // fracPart = p;
            while (p < end && std::isdigit(*p)) ++p;
        }

        // Check for exponent
        if (p < end && (*p == 'e' || *p == 'E')) {
            isFloat = true;
            ++p;
            if (p < end && (*p == '+' || *p == '-')) ++p;
            while (p < end && std::isdigit(*p)) ++p;
        }

        // Update position_
        position_ = p - input_.data();

        // Fast path: pure integer
        if (!isFloat && hasIntDigits) {
            int value;
            auto [ptr, ec] = std::from_chars(start, p, value);
            if (ec == std::errc{} && ptr == p) {
                if (value >= std::numeric_limits<int>::min() &&
                    value <= std::numeric_limits<int>::max()) {
                    return Value(static_cast<int>(value));
                }
                return Value(value);
            }
            // Fall through to double if out of range
        }

        double d;
        const auto [doubleEnd, doubleError] = std::from_chars(start, p, d);
        if (doubleError == std::errc{} && doubleEnd == p) {
            return Value(d);
        }

        // Fallback: invalid number
        std::string numStr(start, p);
        throwException("Invalid number: " + numStr);
    }

    Value parseString() { return Value(parseStringContent()); }

    std::string parseStringContent() {
        expect("\"");
        std::string result;
        result.reserve(64);

        while (position_ < input_.size() && input_[position_] != '"') {
            if (input_[position_] == '\\') {
                ++position_;
                if (position_ >= input_.size()) {
                    throwException("Invalid escape sequence");
                }

                switch (input_[position_]) {
                    case '"':
                        result.push_back('"');
                        break;
                    case '\\':
                        result.push_back('\\');
                        break;
                    case '/':
                        result.push_back('/');
                        break;
                    case 'b':
                        result.push_back('\b');
                        break;
                    case 'f':
                        result.push_back('\f');
                        break;
                    case 'n':
                        result.push_back('\n');
                        break;
                    case 'r':
                        result.push_back('\r');
                        break;
                    case 't':
                        result.push_back('\t');
                        break;
                    case 'u':
                        ParseUnicode(result);
                        break;
                    default:
                        throwException("Invalid escape character");
                }
            } else {
                const size_t runStart = position_;
                do {
                    ++position_;
                } while (position_ < input_.size() && input_[position_] != '"' &&
                         input_[position_] != '\\');
                result.append(input_.data() + runStart, position_ - runStart);
                continue;
            }
            ++position_;
        }

        expect("\"");
        return result;
    }

    void ParseUnicode(std::string& result) {
        position_++;
        if (position_ + 4 >= input_.size()) {
            throwException("Invalid Unicode escape sequence");
        }

        const std::string_view hexStr = input_.substr(position_, 4);
        position_ += 4;

        unsigned int codePoint = 0;
        const auto [codeEnd, codeError] =
            std::from_chars(hexStr.data(), hexStr.data() + hexStr.size(), codePoint, 16);
        if (codeError != std::errc{} || codeEnd != hexStr.data() + hexStr.size()) {
            throwException("Invalid Unicode escape sequence");
        }

        if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
            // Handle surrogate pairs
            if (position_ + 6 >= input_.size() || input_[position_] != '\\' ||
                input_[position_ + 1] != 'u') {
                throwException("Invalid Unicode surrogate pair");
            }
            position_ += 2;
            const std::string_view lowHexStr = input_.substr(position_, 4);
            position_ += 4;
            unsigned int lowCodePoint = 0;
            const auto [lowEnd, lowError] = std::from_chars(
                lowHexStr.data(), lowHexStr.data() + lowHexStr.size(), lowCodePoint, 16);
            if (lowError != std::errc{} || lowEnd != lowHexStr.data() + lowHexStr.size()) {
                throwException("Invalid Unicode surrogate pair");
            }
            if (lowCodePoint < 0xDC00 || lowCodePoint > 0xDFFF) {
                throwException("Invalid Unicode surrogate pair");
            }
            codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowCodePoint - 0xDC00);
        }

        if (codePoint <= 0x7F) {
            result.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7FF) {
            result.push_back(static_cast<char>(0xC0 | ((codePoint >> 6) & 0x1F)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0xFFFF) {
            result.push_back(static_cast<char>(0xE0 | ((codePoint >> 12) & 0x0F)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0x10FFFF) {
            result.push_back(static_cast<char>(0xF0 | ((codePoint >> 18) & 0x07)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            result.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else {
            throwException("Invalid Unicode code point");
        }
        position_--;  // 为了在循环结束后 position_++
    }

    Value parseArray() {
        expect("[");
        Value arrayValue;
        arrayValue.SetArray();

        skipWhitespace();
        if (position_ < input_.size() && input_[position_] == ']') {
            ++position_;
            return arrayValue;
        }

        while (true) {
            arrayValue.Append(parseValue());
            skipWhitespace();
            if (position_ >= input_.size()) throwException("Unexpected end of array");
            if (input_[position_] == ',') {
                ++position_;
                skipWhitespace();
            } else if (input_[position_] == ']') {
                ++position_;
                break;
            } else {
                throwException("Expected ',' or ']'");
            }
        }

        return arrayValue;
    }

    Value parseObject() {
        expect("{");
        Value objectValue;
        objectValue.SetObject();

        skipWhitespace();
        if (position_ < input_.size() && input_[position_] == '}') {
            ++position_;
            return objectValue;
        }

        while (true) {
            if (position_ >= input_.size()) {
                throwException("Unexpected end of object");
            }
            if (input_[position_] != '"') {
                throwException("Expected string key");
            }
            std::string key = parseStringContent();
            skipWhitespace();
            expect(":");
            Value value = parseValue();
            objectValue[std::move(key)] = std::move(value);
            skipWhitespace();
            if (position_ >= input_.size()) {
                throwException("Unexpected end of object");
            }
            if (input_[position_] == ',') {
                ++position_;
                skipWhitespace();
            } else if (input_[position_] == '}') {
                ++position_;
                break;
            } else {
                throwException("Expected ',' or '}'");
            }
        }

        return objectValue;
    }

    // 删除尾随 ","
    static std::string removeTailComma(const std::string& str) {
        /*
            [1,2,] ==> [1,2]
            {"key": "value",} ==> {"key": "value"}
        */
        std::string result;
        result.reserve(str.size());
        bool inString = false;
        bool inArray = false;
        bool inObject = false;
        for (size_t i = 0; i < str.size(); ++i) {
            if (str[i] == '"' && (i == 0 || str[i - 1] != '\\')) {
                inString = !inString;
            }
            if (!inString) {
                if (str[i] == '[') {
                    inArray = true;
                } else if (str[i] == ']') {
                    inArray = false;
                } else if (str[i] == '{') {
                    inObject = true;
                } else if (str[i] == '}') {
                    inObject = false;
                }
                if ((inArray || inObject) && str[i] == ',') {
                    size_t j = i + 1;
                    while (j < str.size() && std::isspace(static_cast<unsigned char>(str[j]))) {
                        ++j;
                    }
                    if (j < str.size() && (str[j] == ']' || str[j] == '}')) {
                        continue;
                    }
                }
            }
            result.push_back(str[i]);
        }
        return result;
    }

    // 去除注释
    static std::string removeComments(const std::string& str) {
        std::string result;
        result.reserve(str.size());
        bool inString = false;
        bool inSingleLineComment = false;
        bool inMultiLineComment = false;

        for (size_t i = 0; i < str.size(); ++i) {
            if (inSingleLineComment) {
                if (str[i] == '\n') {
                    inSingleLineComment = false;
                    result.push_back(str[i]);
                }
            } else if (inMultiLineComment) {
                if (str[i] == '*' && i + 1 < str.size() && str[i + 1] == '/') {
                    inMultiLineComment = false;
                    ++i;
                }
            } else {
                if (str[i] == '"' && (i == 0 || str[i - 1] != '\\' ||
                                      (i >= 2 && str[i - 1] == '\\' && str[i - 2] == '\\'))) {
                    inString = !inString;
                    result.push_back(str[i]);
                } else if (!inString && str[i] == '/' && i + 1 < str.size() && str[i + 1] == '/') {
                    inSingleLineComment = true;
                    ++i;
                } else if (!inString && str[i] == '/' && i + 1 < str.size() && str[i + 1] == '*') {
                    inMultiLineComment = true;
                    ++i;
                } else {
                    result.push_back(str[i]);
                }
            }
        }

        return result;
    }

    void skipWhitespace() {
        while (position_ < input_.size() &&
               std::isspace(static_cast<unsigned char>(input_[position_]))) {
            ++position_;
        }
    }

    void expect(std::string_view expected) {
        if (input_.substr(position_, expected.size()) != expected) {
            throwException("Expected '" + std::string(expected) + "'");
        }
        position_ += expected.size();
    }

    [[noreturn]] void throwException(
        const std::string& message,
        std::source_location func_location = std::source_location::current()) {
        std::string error_msg = message + ": position = ";
        size_t msg_long = 20;
        size_t startIndex = position_ > msg_long ? position_ - msg_long : 0;
        size_t endIndex =
            position_ + msg_long < input_.size() ? position_ + msg_long : input_.size();
        const char current = position_ < input_.size() ? input_[position_] : '\0';
        error_msg += std::to_string(position_) + ": \'" + current + "\' near: \"" +
                     std::string(input_.substr(startIndex, position_ - startIndex)) + "\033[31m" +
                     current + "\033[0m";  // 红色
        if (position_ < input_.size()) {
            error_msg += std::string(input_.substr(position_ + 1, endIndex - position_ - 1));
        }
        error_msg += "\"";
        throw JsonParseException(error_msg, JsonErrorType::UnknownError, func_location);
    }

    std::string_view input_;
    size_t position_;
};
}  // namespace json
}  // namespace bre
