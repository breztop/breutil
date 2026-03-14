#pragma once

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "json_exception.hpp"
#include "json_value.hpp"


namespace bre {
namespace json {
class Generator {
public:
    Generator() = default;

    // 生成 JSON 字符串 (pretty = true 表示生成带缩进的字符串)
    static std::string generate(const Value& val, bool pretty = true, int indentWidth = 2,
                                bool sortKeys = false) {
        Generator generator(pretty ? indentWidth : 0);
        return generator.generateValue(val, 0, sortKeys);
    }

private:
    explicit Generator(int indentWidth) : indentWidth_(indentWidth) {}

    std::string indent(int level) const { return std::string(level * indentWidth_, ' '); }

    std::string generateValue(const Value& val, int level, bool sortKeys = false) {
        switch (val.type()) {
            case Type::Null:
                return "null";
            case Type::Bool:
                return val.AsBool() ? "true" : "false";
            case Type::Int:
                return std::to_string(val.AsInt());
            case Type::Double:
                return std::to_string(val.AsDouble());
            case Type::String:
                return "\"" + escapeString(val.AsString()) + "\"";
            case Type::Array:
                return generateArray(val.AsArray(), level + 1, sortKeys);
            case Type::Object:
                return generateObject(val.AsObject(), level + 1, sortKeys);
            default:
                throw JsonParseException("Invalid Value type");
        }
    }

    std::string generateArray(const Value::Array& array, int level, bool sortKeys) {
        if (array.empty()) return "[]";

        std::string result;
        result.reserve(array.size() * 16);
        result = "[";
        if (indentWidth_ > 0) result += "\n";

        for (size_t i = 0; i < array.size(); ++i) {
            if (indentWidth_ > 0) result += indent(level);
            result += generateValue(array[i], level, sortKeys);
            if (i < array.size() - 1) {
                result += ",";
            }
            if (indentWidth_ > 0) result += "\n";
        }

        if (indentWidth_ > 0) result += indent(level - 1);
        result += "]";
        return result;
    }

    std::string generateObject(const Value::Object& object, int level, bool sortKeys) {
        if (object.empty()) return "{}";

        std::string result;
        result.reserve(object.size() * 32);
        result = "{";
        if (indentWidth_ > 0) result += "\n";

        size_t count = 0;

        if (sortKeys) {
            std::vector<std::string> keys;
            for (const auto& [key, _] : object) {
                keys.push_back(key);
            }
            std::sort(keys.begin(), keys.end());

            for (const auto& key : keys) {
                const auto& value = object.at(key);
                if (indentWidth_ > 0) {
                    result += indent(level);
                    result +=
                        "\"" + escapeString(key) + "\": " + generateValue(value, level, sortKeys);
                } else {
                    result +=
                        "\"" + escapeString(key) + "\":" + generateValue(value, level, sortKeys);
                }

                if (++count < object.size()) {
                    result += ",";
                }
                if (indentWidth_ > 0) result += "\n";
            }
        } else {
            for (const auto& [key, value] : object) {
                if (indentWidth_ > 0) {
                    result += indent(level);
                    result +=
                        "\"" + escapeString(key) + "\": " + generateValue(value, level, sortKeys);
                } else {
                    result +=
                        "\"" + escapeString(key) + "\":" + generateValue(value, level, sortKeys);
                }

                if (++count < object.size()) {
                    result += ",";
                }
                if (indentWidth_ > 0) result += "\n";
            }
        }

        if (indentWidth_ > 0) result += indent(level - 1);
        result += "}";
        return result;
    }

    std::string escapeString(const std::string& s) {
        std::string escaped;
        for (char c : s) {
            switch (c) {
                case '"':
                    escaped += "\\\"";
                    break;
                case '\\':
                    escaped += "\\\\";
                    break;
                case '\b':
                    escaped += "\\b";
                    break;
                case '\f':
                    escaped += "\\f";
                    break;
                case '\n':
                    escaped += "\\n";
                    break;
                case '\r':
                    escaped += "\\r";
                    break;
                case '\t':
                    escaped += "\\t";
                    break;
                default:
                    escaped += c;
                    break;
            }
        }
        return escaped;
    }

    int indentWidth_;
};
}  // namespace json
}  // namespace bre
