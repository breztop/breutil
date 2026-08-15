#pragma once

#include <algorithm>
#include <charconv>
#include <string>
#include <utility>
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
        generator.output_.reserve(generator.initialCapacity(val));
        generator.appendValue(val, 0, sortKeys);
        return std::move(generator.output_);
    }

private:
    explicit Generator(int indentWidth) : indentWidth_(indentWidth) {}

    size_t initialCapacity(const Value& val) const {
        constexpr size_t minimumCapacity = 256;
        if (val.type() == Type::Array) {
            return std::max(minimumCapacity, val.AsArray().size() * 16);
        }
        if (val.type() == Type::Object) {
            return std::max(minimumCapacity, val.AsObject().size() * 24);
        }
        return minimumCapacity;
    }

    void appendIndent(int level) { output_.append(static_cast<size_t>(level * indentWidth_), ' '); }

    void appendInt(int value) {
        char buffer[32];
        const auto [end, error] = std::to_chars(buffer, buffer + sizeof(buffer), value);
        if (error == std::errc{}) {
            output_.append(buffer, end);
        } else {
            output_ += std::to_string(value);
        }
    }

    void appendValue(const Value& val, int level, bool sortKeys) {
        switch (val.type()) {
            case Type::Null:
                output_ += "null";
                break;
            case Type::Bool:
                output_ += val.AsBool() ? "true" : "false";
                break;
            case Type::Int:
                appendInt(val.AsInt());
                break;
            case Type::Double:
                output_ += std::to_string(val.AsDouble());
                break;
            case Type::String:
                appendQuotedString(val.AsString());
                break;
            case Type::Array:
                appendArray(val.AsArray(), level + 1, sortKeys);
                break;
            case Type::Object:
                appendObject(val.AsObject(), level + 1, sortKeys);
                break;
            default:
                throw JsonParseException("Invalid Value type");
        }
    }

    void appendArray(const Value::Array& array, int level, bool sortKeys) {
        output_ += '[';
        if (array.empty()) {
            output_ += ']';
            return;
        }

        if (indentWidth_ > 0) output_ += '\n';
        for (size_t i = 0; i < array.size(); ++i) {
            if (indentWidth_ > 0) appendIndent(level);
            appendValue(array[i], level, sortKeys);
            if (i + 1 < array.size()) output_ += ',';
            if (indentWidth_ > 0) output_ += '\n';
        }
        if (indentWidth_ > 0) appendIndent(level - 1);
        output_ += ']';
    }

    void appendMember(const std::string& key, const Value& value, int level, bool sortKeys) {
        if (indentWidth_ > 0) appendIndent(level);
        appendQuotedString(key);
        output_ += indentWidth_ > 0 ? ": " : ":";
        appendValue(value, level, sortKeys);
    }

    void appendObject(const Value::Object& object, int level, bool sortKeys) {
        output_ += '{';
        if (object.empty()) {
            output_ += '}';
            return;
        }

        if (indentWidth_ > 0) {
            output_ += '\n';
        }
        size_t count = 0;

        if (sortKeys) {
            using Entry = Value::Object::value_type;
            std::vector<const Entry*> entries;
            entries.reserve(object.size());
            for (const auto& entry : object) {
                entries.push_back(&entry);
            }
            std::sort(entries.begin(), entries.end(), [](const Entry* lhs, const Entry* rhs) {
                return lhs->first < rhs->first;
            });

            for (const Entry* entry : entries) {
                appendMember(entry->first, entry->second, level, sortKeys);
                if (++count < object.size()) output_ += ',';
                if (indentWidth_ > 0) output_ += '\n';
            }
        } else {
            for (const auto& [key, value] : object) {
                appendMember(key, value, level, sortKeys);
                if (++count < object.size()) output_ += ',';
                if (indentWidth_ > 0) output_ += '\n';
            }
        }

        if (indentWidth_ > 0) appendIndent(level - 1);
        output_ += '}';
    }

    void appendQuotedString(const std::string& value) {
        output_ += '"';
        size_t runStart = 0;
        for (size_t i = 0; i < value.size(); ++i) {
            const char* replacement = nullptr;
            switch (value[i]) {
                case '"':
                    replacement = "\\\"";
                    break;
                case '\\':
                    replacement = "\\\\";
                    break;
                case '\b':
                    replacement = "\\b";
                    break;
                case '\f':
                    replacement = "\\f";
                    break;
                case '\n':
                    replacement = "\\n";
                    break;
                case '\r':
                    replacement = "\\r";
                    break;
                case '\t':
                    replacement = "\\t";
                    break;
                default:
                    break;
            }
            if (replacement != nullptr) {
                output_.append(value.data() + runStart, i - runStart);
                output_ += replacement;
                runStart = i + 1;
            }
        }
        output_.append(value.data() + runStart, value.size() - runStart);
        output_ += '"';
    }

    int indentWidth_;
    std::string output_;
};

}  // namespace json
}  // namespace bre
