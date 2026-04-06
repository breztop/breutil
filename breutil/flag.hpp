#pragma once

/*
命令行参数解析库 - C++20 实现
仅支持 “-param=value”, or “-param value” 格式, 不支持 GNU 风格的 --param=value
使用方法：
    Flag flag(argc, argv, "program description");

    // 注册标志
    flag.Add("verbose", false, "Enable verbose output");
    flag.Add("port", 8080, "Server port");
    flag.Add("config", std::string("default.json"), "Config file", true);  // 必需参数

    // 解析命令行参数
    flag.Parse();

    // 获取值（类型安全）
    bool verbose;
    int port;
    std::string config;
    flag.Get("verbose", verbose);
    flag.Get("port", port);
    flag.Get("config", config);

    auto args = flag.Arg();  // 获取非标志参数

命令行格式：
    yourProgram -flag1=value1 -flag2 value2 -bool_flag file1 file2 ...

支持的类型：
    - bool: true/false, t/f, T/F, 1/0
    - 整数: int, int64_t, unsigned int 等
    - 浮点: float, double
    - 字符串: std::string, const char*
*/

// brez 2025.01.05  go flag库的c++实现
// brez 2025.10.19  完全重新构建，使用C++20 concepts重构，简化模板代码，使用variant管理类型


#include <cinttypes>
#include <concepts>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace bre {

template <typename T>
concept BoolType = std::same_as<T, bool>;

template <typename T>
concept IntegralType = std::integral<T> && !std::same_as<T, bool> && !std::same_as<T, char>;

template <typename T>
concept FloatingType = std::floating_point<T>;

template <typename T>
concept StringType = std::same_as<std::remove_cvref_t<T>, std::string> ||
                     std::same_as<std::remove_cvref_t<T>, const char*>;

template <typename T>
concept FlagValueType = BoolType<T> || IntegralType<T> || FloatingType<T> || StringType<T>;

enum class TypeFlag { Bool, Int, Double, String };
std::string ToString(TypeFlag type) {
    switch (type) {
        case TypeFlag::Bool:
            return "bool";
        case TypeFlag::Int:
            return "int";
        case TypeFlag::Double:
            return "double";
        case TypeFlag::String:
            return "string";
        default:
            return "unknown";
    }
}

struct FlagInfo {
    using Shared = std::shared_ptr<FlagInfo>;
    using ValueType = std::variant<bool, int64_t, double, std::string>;

    FlagInfo(const std::string& Name, TypeFlag Type, const std::string& Help, ValueType V,
             ValueType Default, bool IsRequired = false) {
        name = Name;
        type = Type;
        help = Help;
        value = V;
        defValue = Default;
        isRequired = IsRequired;
    }

    void Set(ValueType str) { value = str; }

    std::string ToString() const {
        std::ostringstream oss;
        oss << "Flag{name=" << name << ", type=";
        oss << bre::ToString(type);
        oss << ", value=";
        std::visit(
            [&oss](const auto& v) {
                oss << v;
            },
            value);

        oss << ", default=";
        std::visit(
            [&oss](const auto& v) {
                oss << v;
            },
            defValue);

        oss << ", required=" << (isRequired ? "true" : "false") << "}";
        return oss.str();
    }

    std::string name;
    std::string help;
    TypeFlag type;
    ValueType value;
    ValueType defValue;
    bool isRequired = false;
};

class Flag {
public:
    // 参数个数， 参数， 程序描述
    Flag(int argc, char** argv, const std::string& description = "");
    ~Flag() = default;

    // 添加标志定义（先注册）
    template <typename T>
        requires FlagValueType<T>
    void Add(const std::string& name, T default_value, const std::string& help,
             bool required = false);

    // 解析命令行参数
    void Parse();

    // 获取解析后的值
    template <typename T>
        requires FlagValueType<T>
    void Get(const std::string& name, T& value) const;


    int Narg();                      // 非标志参数数量
    int Nflag();                     // 参数数量
    std::string Arg(int i);          // 获取第i个非标志参数
    std::vector<std::string> Arg();  // 获取非标志参数
    std::vector<FlagInfo::Shared> VisiedAll() const { return _flags; }
    std::string ProgramName() const { return _program_name; }
    std::string Description() const { return _description; }


    bool Parsed() const;  // 是否解析完成

    // 打印帮助信息
    void PrintDefault() const;

    // 设置版本信息
    void SetVersion(const std::string& version);

    // 打印版本信息
    void PrintVersion() const;

private:
    // 解析命令行参数
    void addFlag(FlagInfo::Shared flag_info);

    static bool judgeSuitableValue(TypeFlag type, const std::string& val);
    static bool stringToBool(const std::string& val);

private:
    int _argc;                  // 参数个数
    char** _argv;               // 参数数组
    std::string _program_name;  // 程序名称
    std::string _description;   // 程序描述
    const std::string _prefix_chars{"-"};
    const std::string _assign_chars{"="};
    bool _is_parsed = false;    // 是否已解析

    std::string _version;       // 版本信息

    std::vector<FlagInfo::Shared> _flags;
    std::map<std::string, FlagInfo::Shared> _flag_map;

    std::vector<std::string> _positional_args;
};

inline Flag::Flag(int argc, char** argv, const std::string& description)
    : _argc(argc)
    , _argv(argv)
    , _description(description) {
    if (argc > 0) {
        _program_name = argv[0];
    }
}

inline void Flag::addFlag(FlagInfo::Shared flag_info) {
    _flags.push_back(flag_info);
    _flag_map[flag_info->name] = flag_info;
}

inline void Flag::SetVersion(const std::string& version) { _version = version; }

inline void Flag::PrintVersion() const {
    if (!_version.empty()) {
        std::cout << _program_name << " version " << _version << std::endl;
    } else {
        std::cout << _program_name << " (version not set)" << std::endl;
    }
}
// ============= Add 方法 =============
template <typename T>
    requires FlagValueType<T>
inline void Flag::Add(const std::string& name, T default_value, const std::string& help,
                      bool required) {
    FlagInfo::Shared flagShared;

    if constexpr (BoolType<T>) {
        flagShared = std::make_shared<FlagInfo>(name, TypeFlag::Bool, help, default_value,
                                                default_value, required);
    } else if constexpr (IntegralType<T>) {
        flagShared = std::make_shared<FlagInfo>(name, TypeFlag::Int, help,
                                                static_cast<int64_t>(default_value),
                                                static_cast<int64_t>(default_value), required);
    } else if constexpr (FloatingType<T>) {
        flagShared = std::make_shared<FlagInfo>(name, TypeFlag::Double, help,
                                                static_cast<double>(default_value),
                                                static_cast<double>(default_value), required);
    } else if constexpr (std::same_as<T, std::string>) {
        flagShared = std::make_shared<FlagInfo>(name, TypeFlag::String, help, default_value,
                                                default_value, required);
    } else if constexpr (std::same_as<T, const char*>) {
        std::string def_val(default_value);
        flagShared =
            std::make_shared<FlagInfo>(name, TypeFlag::String, help, def_val, def_val, required);
    }

    addFlag(flagShared);
}

// ============= Parse 方法：统一解析 =============
inline void Flag::Parse() {
    if (_is_parsed) {
        return;
    }

    // 遍历命令行参数
    for (int i = 1; i < _argc; ++i) {
        std::string arg(_argv[i]);

        // 处理 --help 或 -h
        if (arg == "--help" || arg == "-h") {
            PrintDefault();
            exit(EXIT_SUCCESS);
        }

        // 处理 --version 或 -v
        if (arg == "--version" || arg == "-v") {
            PrintVersion();
            exit(EXIT_SUCCESS);
        }

        // 跳过非标志参数
        if (arg.find(_prefix_chars) != 0) {
            _positional_args.push_back(arg);
            continue;
        }

        // 处理前缀
        size_t prefix_pos = 1;
        if (arg.find(_prefix_chars + _prefix_chars) == 0) {
            prefix_pos = _prefix_chars.length() * 2;
        }

        // 查找等号
        size_t eq_pos = arg.find(_assign_chars);
        std::string flag_name;
        std::string value;

        if (eq_pos != std::string::npos) {
            // 格式: -flag=value 或 --flag=value
            flag_name = arg.substr(prefix_pos, eq_pos - prefix_pos);
            value = arg.substr(eq_pos + 1);
        } else {
            // 格式: -flag value 或 -flag (for bool)
            flag_name = arg.substr(prefix_pos);
        }

        // 检查标志是否已注册
        auto it = _flag_map.find(flag_name);
        if (it == _flag_map.end()) {
            std::cerr << "Unknown flag: -" << flag_name << std::endl;
            PrintDefault();
            exit(EXIT_FAILURE);
        }

        auto& flag = it->second;

        // 如果没有通过=赋值，需要获取下一个参数
        if (eq_pos == std::string::npos) {
            if (flag->type == TypeFlag::Bool) {
                // 布尔类型特殊处理
                if (i + 1 >= _argc || _argv[i + 1][0] == '-') {
                    value = "true";
                } else {
                    std::string next_val(_argv[i + 1]);
                    if (judgeSuitableValue(TypeFlag::Bool, next_val)) {
                        value = stringToBool(next_val) ? "true" : "false";
                        i++;  // 消耗下一个参数
                    } else {
                        value = "true";
                    }
                }
            } else {
                // 其他类型必须有值
                if (i + 1 >= _argc) {
                    std::cerr << "Flag -" << flag_name << " requires a value" << std::endl;
                    PrintDefault();
                    exit(EXIT_FAILURE);
                }
                value = _argv[i + 1];
                i++;  // 消耗下一个参数
            }
        }

        // 验证值的有效性
        if (!judgeSuitableValue(flag->type, value)) {
            std::cerr << "Invalid value '" << value << "' for flag -" << flag_name << std::endl;
            PrintDefault();
            exit(EXIT_FAILURE);
        }

        // 设置值
        switch (flag->type) {
            case TypeFlag::Bool:
                flag->Set(stringToBool(value));
                break;
            case TypeFlag::Int:
                flag->Set(std::stoll(value));
                break;
            case TypeFlag::Double:
                flag->Set(std::stod(value));
                break;
            case TypeFlag::String:
                flag->Set(value);
                break;
        }
    }

    _is_parsed = true;

    // 检查所有必需的标志是否都已设置
    for (const auto& flag : _flags) {
        if (flag->isRequired) {
            // 检查值是否与默认值相同，如果相同说明未被设置
            bool is_default = false;
            std::visit(
                [&is_default, &flag](const auto& current_val) {
                    using T = std::decay_t<decltype(current_val)>;
                    const T& default_val = std::get<T>(flag->defValue);
                    is_default = (current_val == default_val);
                },
                flag->value);

            if (is_default) {
                std::cerr << "Error: Required flag -" << flag->name << " is missing" << std::endl;
                PrintDefault();
                exit(EXIT_FAILURE);
            }
        }
    }
}

// ============= Get 方法 =============
template <typename T>
    requires FlagValueType<T>
inline void Flag::Get(const std::string& name, T& value) const {
    auto it = _flag_map.find(name);
    if (it == _flag_map.end()) {
        std::cerr << "Flag not found: " << name << std::endl;
        exit(EXIT_FAILURE);
    }

    if constexpr (BoolType<T>) {
        value = std::get<bool>(it->second->value);
    } else if constexpr (IntegralType<T>) {
        value = static_cast<T>(std::get<int64_t>(it->second->value));
    } else if constexpr (FloatingType<T>) {
        value = static_cast<T>(std::get<double>(it->second->value));
    } else if constexpr (StringType<T>) {
        value = std::get<std::string>(it->second->value);
    }
}

// 仅判断传入的值是否符合当前类型的预期
// static
inline bool Flag::judgeSuitableValue(TypeFlag type, const std::string& val) {
    // 字符串类型允许空值
    if (type == TypeFlag::String) {
        return true;
    }

    // 其他类型不允许空值
    if (val.empty()) {
        return false;
    }

    if (type == TypeFlag::Bool) {  // 1, 0, t, f, T, F, true, false, TRUE, FALSE, True, False
        if (val == "1" || val == "0" || val == "t" || val == "f" || val == "T" || val == "F" ||
            val == "true" || val == "false" || val == "TRUE" || val == "FALSE" || val == "True" ||
            val == "False") {
            return true;
        }
    }

    try {
        if (type == TypeFlag::Int) {
            std::stoll(val);
            return true;
        }

        if (type == TypeFlag::Double) {
            std::stod(val);
            return true;
        }
    } catch (const std::exception&) {
        return false;
    }

    return false;
}


// 传入字符必须是1, 0, t, f, T, F, true, false, TRUE, FALSE, True, False
// static
inline bool Flag::stringToBool(const std::string& val) {
    if (val == "1" || val == "t" || val == "T" || val == "true" || val == "TRUE" || val == "True") {
        return true;
    } else if (val == "0" || val == "f" || val == "F" || val == "false" || val == "FALSE" ||
               val == "False") {
        return false;
    }

    return false;
}

inline int Flag::Narg() {
    if (_positional_args.empty()) {
        // 懒加载位置参数
        for (int i = 1; i < _argc; ++i) {
            std::string arg(_argv[i]);
            if (arg.find(_prefix_chars) != 0) {
                _positional_args.push_back(arg);
            }
        }
    }
    return static_cast<int>(_positional_args.size());
}

inline int Flag::Nflag() { return static_cast<int>(_flags.size()); }

inline std::string Flag::Arg(int i) {
    Narg();  // 确保位置参数已解析
    if (i > 0 && i <= static_cast<int>(_positional_args.size())) {
        return _positional_args[i - 1];
    }
    return "";
}

inline std::vector<std::string> Flag::Arg() {
    Narg();  // 确保位置参数已解析
    return _positional_args;
}

inline bool Flag::Parsed() const { return _is_parsed; }

inline void Flag::PrintDefault() const {
    // 打印程序描述
    if (!_description.empty()) {
        std::cout << _description << "\n" << std::endl;
    }

    if (_flags.empty()) {
        return;
    }

    std::cout << "Usage: " << _program_name << " [OPTIONS]" << std::endl;
    std::cout << "\nOptions:" << std::endl;

    // 计算最大名称长度，用于对齐
    size_t max_name_len = 0;
    for (const auto& flag : _flags) {
        size_t len =
            flag->name.length() + bre::ToString(flag->type).length() + 3;  // "-name <type>"
        if (len > max_name_len) {
            max_name_len = len;
        }
    }

    // 打印每个标志
    for (const auto& flag : _flags) {
        std::ostringstream flag_str;
        flag_str << "  -" << flag->name << " <" << bre::ToString(flag->type) << ">";

        std::string flag_name = flag_str.str();
        std::cout << flag_name;

        // 计算需要的空格数来对齐
        size_t padding = max_name_len + 5 - flag_name.length();
        std::cout << std::string(padding, ' ');

        // 打印帮助信息
        std::cout << flag->help;

        // 如果是必需参数，标记 [required]
        if (flag->isRequired) {
            std::cout << " [required]";
        } else {
            // 打印默认值（仅对非必需参数）
            std::cout << " (default: ";
            std::visit(
                [](const auto& v) {
                    using T = std::decay_t<decltype(v)>;
                    if constexpr (std::is_same_v<T, bool>) {
                        std::cout << (v ? "true" : "false");
                    } else {
                        std::cout << v;
                    }
                },
                flag->defValue);
            std::cout << ")";
        }

        std::cout << std::endl;
    }
}

}  // namespace bre