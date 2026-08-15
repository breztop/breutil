#pragma once

#include "json_exception.hpp"
#include "json_generator.hpp"
#include "json_parse.hpp"
#include "json_value.hpp"


namespace bre {
namespace json {
inline std::ostream& operator<<(std::ostream& os, const Value& val) {
    os << Generator::generate(val);
    return os;
}

// 生成 JSON 字符串，不带缩进
inline std::string Value::ToString(bool need_indent, int indent, bool sortKeys) const {
    return json::Generator::generate(*this, need_indent, indent, sortKeys);
}


// 递归比较两个 Value 对象是否相等
inline bool Value::operator==(const Value& other) const {
    if (type_ != other.type_) {
        return false;
    }

    switch (type_) {
        case Type::Null:
            return true;
        case Type::Int:
            return this->AsInt() == other.AsInt();
        case Type::Double:
            return this->AsDouble() == other.AsDouble();
        case Type::Bool:
            return this->AsBool() == other.AsBool();
        case Type::String:
            return this->AsString() == other.AsString();
        case Type::Array: {
            const auto& thisArray = this->AsArray();
            const auto& otherArray = other.AsArray();
            if (thisArray.size() != otherArray.size()) {
                return false;
            }
            for (size_t i = 0; i < thisArray.size(); ++i) {
                if (!thisArray[i].operator==(otherArray[i])) {
                    return false;
                }
            }
            return true;
        }
        case Type::Object: {
            const auto& thisObject = this->AsObject();
            const auto& otherObject = other.AsObject();
            if (thisObject.size() != otherObject.size()) {
                return false;
            }
            return thisObject == otherObject;
        }
        default:
            return false;
    }
    return false;
}


}  // namespace json
}  // namespace bre
