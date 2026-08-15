#pragma once

#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json_exception.hpp"

namespace bre {

namespace json {
class ConstValueIterator;
class ValueIterator;

class Generator;

enum class Type { Null, Bool, Int, Double, String, Array, Object };

struct TransparentStringHash {
    using is_transparent = void;

    size_t operator()(std::string_view value) const noexcept {
        return std::hash<std::string_view>{}(value);
    }
};

class Value {
public:
    using Object =
        std::unordered_map<std::string, Value, TransparentStringHash, std::equal_to<>>;
    using Array = std::vector<Value>;

    static std::string TypeStr(Type type) {
        switch (type) {
            case Type::Null:
                return "Null";
            case Type::Bool:
                return "Bool";
            case Type::Int:
                return "Int";
            case Type::Double:
                return "Double";
            case Type::String:
                return "String";
            case Type::Array:
                return "Array";
            case Type::Object:
                return "Object";
            default:
                return "Unknown";
        }
    }


    Value() : type_(Type::Null), value_(std::monostate{}) {}
    Value(const Value& other) : type_(other.type_), value_(other.value_) {}
    Value(Value&& other) noexcept : type_(other.type_), value_(std::move(other.value_)) {}

    ~Value() = default;

    explicit Value(bool b) : type_(Type::Bool), value_(b) {}
    Value(int i) : type_(Type::Int), value_(i) {}
    explicit Value(double d) : type_(Type::Double), value_(d) {}
    Value(const char* s) : type_(Type::String), value_(std::string(s)) {}
    Value(const std::string& s) : type_(Type::String), value_(s) {}
    Value(std::string&& s) noexcept : type_(Type::String), value_(std::move(s)) {}
    Value(const Array& arr) : type_(Type::Array), value_(arr) {}
    Value(Array&& arr) noexcept : type_(Type::Array), value_(std::move(arr)) {}
    Value(const Object& obj) : type_(Type::Object), value_(obj) {}
    Value(Object&& obj) noexcept : type_(Type::Object), value_(std::move(obj)) {}

    Value& operator=(const Value& other) {
        if (this == &other) {
            return *this;
        }
        type_ = other.type_;
        value_ = other.value_;
        return *this;
    }

    Value& operator=(Value&& other) noexcept {
        if (this != &other) {
            type_ = other.type_;
            value_ = std::move(other.value_);
        }
        return *this;
    }

    // 重载比较运算符
    bool operator==(const Value& other) const;

    Type type() const { return type_; }

    void Clear() {
        type_ = Type::Null;
        value_ = std::monostate{};
    }

    void SetNull() { Clear(); }

    void SetBool(bool b) {
        type_ = Type::Bool;
        value_ = b;
    }

    void SetInt(int i) {
        type_ = Type::Int;
        value_ = i;
    }

    void SetDouble(double d) {
        type_ = Type::Double;
        value_ = d;
    }

    void SetString(const std::string& s) {
        type_ = Type::String;
        value_ = s;
    }

    void SetString(std::string&& s) {
        type_ = Type::String;
        value_ = std::move(s);
    }

    void SetArray() {
        type_ = Type::Array;
        value_ = Array{};
        get<Array>(value_).reserve(8);
    }

    void SetObject() {
        type_ = Type::Object;
        value_ = Object{};
        get<Object>(value_).reserve(8);
    }

    // 类型转换函数
    bool AsBool() const {
        checkType(Type::Bool);
        return std::get<bool>(value_);
    }

    int AsInt() const {
        checkType(Type::Int);
        return std::get<int>(value_);
    }

    double AsDouble() const {
        checkType(Type::Double);
        return std::get<double>(value_);
    }

    std::string& AsString() {
        checkType(Type::String);
        return std::get<std::string>(value_);
    }

    const std::string& AsString() const {
        checkType(Type::String);
        return std::get<std::string>(value_);
    }

    Array& AsArray() {
        checkType(Type::Array);
        return std::get<Array>(value_);
    }
    const Array& AsArray() const {
        checkType(Type::Array);
        return std::get<Array>(value_);
    }

    Object& AsObject() {
        checkType(Type::Object);
        return std::get<Object>(value_);
    }


    const Object& AsObject() const {
        checkType(Type::Object);
        return std::get<Object>(value_);
    }

    bool IsNull() const { return type_ == Type::Null; }

    bool IsBool() const { return type_ == Type::Bool; }

    bool IsInt() const { return type_ == Type::Int; }

    bool IsUInt() const { return IsInt() && AsInt() >= 0; }

    bool IsIntegral() const { return IsInt() || IsUInt(); }

    bool IsDouble() const { return type_ == Type::Double; }

    bool IsNumeric() const { return IsIntegral() || IsDouble(); }

    bool IsString() const { return type_ == Type::String; }

    bool IsArray() const { return type_ == Type::Array; }

    bool IsObject() const { return type_ == Type::Object; }

    bool IsConvertibleTo(Type other) const {
        switch (other) {
            case Type::Null:
                return IsNull();
            case Type::Bool:
                return IsBool();
            case Type::Int:
                return IsInt();
            case Type::Double:
                return IsNumeric();
            case Type::String:
                return IsString();
            case Type::Array:
                return IsArray();
            case Type::Object:
                return IsObject();
            default:
                return false;
        }
    }

    bool Empty() const {
        switch (type_) {
            case Type::Null:
                return true;
            case Type::Bool:
                return false;
            case Type::Int:
                return false;
            case Type::Double:
                return false;
            case Type::String:
                return std::get<std::string>(value_).empty();
            case Type::Array:
                return std::get<Array>(value_).empty();
            case Type::Object:
                return std::get<Object>(value_).empty();
            default:
                return true;
        }
    }


    void Resize(int newSize) {
        checkType(Type::Array);
        std::get<Array>(value_).resize(newSize);
    }

    // 数组和对象访问
    Value& operator[](size_t index) {
        checkType(Type::Array);
        Array& arr = std::get<Array>(value_);
        if (index >= arr.size()) {
            arr.resize(index + 1);
        }
        return arr[index];
    }

    Value& operator[](const std::string& key) {
        if (type_ == Type::Null) {
            SetObject();
        }
        checkType(Type::Object);
        return std::get<Object>(value_)[key];
    }

    Value& operator[](std::string&& key) {
        if (type_ == Type::Null) {
            SetObject();
        }
        checkType(Type::Object);
        return std::get<Object>(value_)[std::move(key)];
    }

    const Value& operator[](size_t index) const {
        checkType(Type::Array);
        const Array& arr = std::get<Array>(value_);
        if (index >= arr.size()) {
            throw std::out_of_range("Array index out of range");
        }
        return arr[index];
    }

    const Value& operator[](const std::string& key) const {
        checkType(Type::Object);
        const Object& obj = std::get<Object>(value_);
        auto it = obj.find(key);
        if (it == obj.end()) {
            static const Value emptyValue;
            return emptyValue;  // 返回一个空值
        }
        return it->second;
    }

    Value& At(size_t index) {
        checkType(Type::Array);
        return std::get<Array>(value_).at(index);
    }

    const Value& At(size_t index) const {
        checkType(Type::Array);
        return std::get<Array>(value_).at(index);
    }

    Value& At(std::string_view key) {
        checkType(Type::Object);
        auto& object = std::get<Object>(value_);
        const auto it = object.find(key);
        if (it == object.end()) {
            throw std::out_of_range("Object key not found");
        }
        return it->second;
    }

    const Value& At(std::string_view key) const {
        checkType(Type::Object);
        const auto& object = std::get<Object>(value_);
        const auto it = object.find(key);
        if (it == object.end()) {
            throw std::out_of_range("Object key not found");
        }
        return it->second;
    }

    void Append(const Value& val) {
        checkType(Type::Array);
        std::get<Array>(value_).push_back(val);
    }

    void Append(Value&& val) {
        checkType(Type::Array);
        std::get<Array>(value_).push_back(std::move(val));
    }

    void Reserve(size_t capacity) {
        if (type_ == Type::Array) {
            std::get<Array>(value_).reserve(capacity);
        } else if (type_ == Type::Object) {
            std::get<Object>(value_).reserve(capacity);
        } else {
            checkType(Type::Array);
        }
    }

    void Remove(size_t index) {
        checkType(Type::Array);
        Array& arr = std::get<Array>(value_);
        if (index >= arr.size()) {
            throw std::out_of_range("Array index out of range");
        }
        arr.erase(arr.begin() + index);
    }

    void Remove(const std::string& key) {
        checkType(Type::Object);
        Object& obj = std::get<Object>(value_);
        obj.erase(key);
    }

    size_t Size() const {
        switch (type_) {
            case Type::Array:
                return std::get<Array>(value_).size();
            case Type::Object:
                return std::get<Object>(value_).size();
            default:
                return 0;
        }
    }

    std::string GetTypeStr() const { return TypeStr(type_); }

    std::string ToString(bool need_indent = false, int indent = 2, bool sortKeys = false) const;

    ValueIterator begin();

    ValueIterator end();

    ConstValueIterator begin() const;

    ConstValueIterator end() const;

private:
    Type type_;
    std::variant<std::monostate, bool, int, double, std::string, Array, Object> value_;


    void checkType(Type expected) const {
        if (type_ != expected) {
            throw JsonParseException::TypeError("Invalid type");
        }
    }
};


class ValueIterator {
public:
    using ArrayIterator = Value::Array::iterator;
    using ObjectIterator = Value::Object::iterator;

    ValueIterator() = default;

    explicit ValueIterator(ArrayIterator arrayIt) : arrayIt_(arrayIt), isArray_(true) {}
    explicit ValueIterator(ObjectIterator objectIt) : objectIt_(objectIt), isArray_(false) {}

    Value& operator*() {
        if (isArray_) {
            return *arrayIt_;
        } else {
            return objectIt_->second;
        }
    }

    Value* operator->() {
        if (isArray_) {
            return &(*arrayIt_);
        } else {
            return &(objectIt_->second);
        }
    }

    ValueIterator& operator++() {
        if (isArray_) {
            ++arrayIt_;
        } else {
            ++objectIt_;
        }
        return *this;
    }

    ValueIterator operator++(int) {
        ValueIterator temp = *this;
        ++(*this);
        return temp;
    }


    bool operator==(const ValueIterator& other) const {
        return isArray_ == other.isArray_ &&
               (isArray_ ? arrayIt_ == other.arrayIt_ : objectIt_ == other.objectIt_);
    }

    bool operator!=(const ValueIterator& other) const { return !(*this == other); }

private:
    ArrayIterator arrayIt_;
    ObjectIterator objectIt_;
    bool isArray_ = true;
};

class ConstValueIterator {
public:
    using ArrayConstIterator = Value::Array::const_iterator;
    using ObjectConstIterator = Value::Object::const_iterator;

    ConstValueIterator() = default;

    explicit ConstValueIterator(ArrayConstIterator arrayIt) : arrayIt_(arrayIt), isArray_(true) {}
    explicit ConstValueIterator(ObjectConstIterator objectIt)
        : objectIt_(objectIt)
        , isArray_(false) {}

    const Value& operator*() const {
        if (isArray_) {
            return *arrayIt_;
        } else {
            return objectIt_->second;
        }
    }

    const Value* operator->() const {
        if (isArray_) {
            return &(*arrayIt_);
        } else {
            return &(objectIt_->second);
        }
    }

    ConstValueIterator& operator++() {
        if (isArray_) {
            ++arrayIt_;
        } else {
            ++objectIt_;
        }
        return *this;
    }

    ConstValueIterator operator++(int) {
        ConstValueIterator temp = *this;
        ++(*this);
        return temp;
    }

    bool operator==(const ConstValueIterator& other) const {
        return isArray_ == other.isArray_ &&
               (isArray_ ? arrayIt_ == other.arrayIt_ : objectIt_ == other.objectIt_);
    }

    bool operator!=(const ConstValueIterator& other) const { return !(*this == other); }

private:
    ArrayConstIterator arrayIt_;
    ObjectConstIterator objectIt_;
    bool isArray_ = true;
};

// 在 Value 类中实现 begin() 和 end() 方法
inline ValueIterator Value::begin() {
    if (type_ == Type::Array) {
        return ValueIterator(std::get<Array>(value_).begin());
    } else if (type_ == Type::Object) {
        return ValueIterator(std::get<Object>(value_).begin());
    } else {
        std::string cur_type = GetTypeStr();
        std::string err_msg = "Value: " + cur_type + " is not iterable";
        throw JsonParseException::TypeError(err_msg);
    }
}

inline ValueIterator Value::end() {
    if (type_ == Type::Array) {
        return ValueIterator(std::get<Array>(value_).end());
    } else if (type_ == Type::Object) {
        return ValueIterator(std::get<Object>(value_).end());
    } else {
        std::string cur_type = GetTypeStr();
        std::string err_msg = "Value: " + cur_type + "is not iterable";
        throw JsonParseException::TypeError(err_msg);
    }
}

inline ConstValueIterator Value::begin() const {
    if (type_ == Type::Array) {
        return ConstValueIterator(std::get<Array>(value_).begin());
    } else if (type_ == Type::Object) {
        return ConstValueIterator(std::get<Object>(value_).begin());
    } else {
        std::string cur_type = GetTypeStr();
        std::string err_msg = "Value: " + cur_type + "is not iterable";
        throw JsonParseException::TypeError(err_msg);
    }
}

inline ConstValueIterator Value::end() const {
    if (type_ == Type::Array) {
        return ConstValueIterator(std::get<Array>(value_).end());
    } else if (type_ == Type::Object) {
        return ConstValueIterator(std::get<Object>(value_).end());
    } else {
        std::string cur_type = GetTypeStr();
        std::string err_msg = "Value: " + cur_type + "is not iterable";
        throw JsonParseException::TypeError(err_msg);
    }
}


}  // namespace json
}  // namespace bre
