/**
 * @file config.hpp
 * @brief 一个灵活的配置管理类，支持结构化key-value存储、验证、序列化等功能
 * @note 支持的类型：
 * - 基础类型：bool, int, double, string, enum class(使用int存储)等
 * - 容器类型：std::shared_ptr<T>, std::weak_ptr<T>
 * - 函数类型：std::function<ReturnType(Args...)>
 * - 用户自定义类型: 需要copyable
 * @details
 * Config 类是一个单例模式的配置管理系统，提供以下核心功能：
 * - 支持点号（.）分隔的层级结构化配置访问
 * - 类型安全的注册、设置和获取操作
 * - 支持自定义验证函数进行值的合法性检查
 * - 支持序列化到JSON和从JSON反序列化 (非函数对象等不可序列化类型)
 * - 支持共享指针（shared_ptr）和函数对象的存储, 不支持unique_ptr等非copyable类型
 * - 支持变更回调通知机制
 *
 * @note 使用方法：
 * 1. 注册配置：先通过 Register() 注册配置项及其验证函数
 * 2. 结构化访问：使用 "模块.子模块1.子模块2.配置项" 形式访问配置
 * 3. 设置值：使用 Set() 设置配置值，会自动执行验证
 * 4. 获取值：使用 Get() 获取配置值，支持模板参数推导
 * 5. 序列化：支持 ToJson()/FromJson() 进行JSON格式转换
 * 6. 变更通知：使用 NotifyChange() 注册回调函数监听配置变化
 *
 * @note 重要限制：
 * - 所有key必须先注册，才能被设置，未注册的key会抛出异常
 * - 每个key的单个部分不能含有空格、点号，且不能以"__"开头
 * - 不允许存储原始指针，必须使用共享指针（shared_ptr）
 * - 相同的key注册第二次会覆盖第一次的配置
 *
 * @note
 * 使用 BRE_CONFIG_JSON_SUPPORT 宏定义来启用或禁用JSON序列化功能，默认关闭
 *
 */

#pragma once
#include <any>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>


#ifdef BRE_CONFIG_JSON_SUPPORT
#include "json.hpp"
#endif
#include "singleton.hpp"
#include "string_util.hpp"

namespace bre {

namespace config_detail {

// 类型特征:判断是否为字符串类型
template <typename T>
struct is_string_like : std::false_type {};

template <>
struct is_string_like<std::string> : std::true_type {};

template <>
struct is_string_like<const char*> : std::true_type {};

template <>
struct is_string_like<char*> : std::true_type {};

template <size_t N>
struct is_string_like<const char[N]> : std::true_type {};

template <size_t N>
struct is_string_like<char[N]> : std::true_type {};

// 类型特征:判断是否为非序列化类型（共享指针、函数等）
template <typename T>
struct is_non_serializable : std::false_type {};

// 共享指针类型
template <typename T>
struct is_non_serializable<std::shared_ptr<T>> : std::true_type {};

template <typename T>
struct is_non_serializable<std::weak_ptr<T>> : std::true_type {};

// 函数类型
template <typename Ret, typename... Args>
struct is_non_serializable<std::function<Ret(Args...)>> : std::true_type {};

// 类型归一化:将字符串类型统一为 std::string，枚举类型统一为 int，函数类型保持原样
template <typename T>
using normalize_type = std::conditional_t<
    is_string_like<std::remove_cv_t<std::remove_reference_t<T>>>::value, std::string,
    std::conditional_t<std::is_enum_v<std::remove_cv_t<std::remove_reference_t<T>>>, int,
                       std::remove_cv_t<std::remove_reference_t<T>>>>;
}  // namespace config_detail


class Config : public Singleton<Config> {
    friend class Singleton<Config>;

private:
    // 值转换:将任意类型转为规范化类型
    template <typename T>
    auto normalize_value(T&& value) {
        using namespace config_detail;

        using normalized = normalize_type<T>;
        if constexpr (std::is_same_v<normalized, std::string>) {
            // 处理空指针情况
            if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) {
                if (value == nullptr) {
                    return std::string();
                }
            }
            return std::string(std::forward<T>(value));
        } else if constexpr (is_non_serializable<std::remove_reference_t<T>>::value) {
            // 非序列化类型直接转发
            return std::forward<T>(value);
        } else if constexpr (std::is_enum_v<std::remove_reference_t<T>>) {
            // 枚举类型转为 int 存储
            return static_cast<int>(value);
        } else {
            return std::forward<T>(value);
        }
    }

    const char* getClassName() const override { return "bre::Config"; }

public:
    /**
     * @brief 注册配置项，指定自定义验证/转换函数
     * @tparam T 配置值的类型，支持基础类型、共享指针和函数
     * @param keys 配置项的分层键，使用 "." 分隔，如 "video.fps"
     * @param validator 验证/转换函数，签名为 T(bool is_set, T value)
     *                  - is_set=false 时返回默认值
     *                  - is_set=true 时进行值的验证和转换
     *                  - 返回经过验证/转换后的值
     */
    template <typename T>
    void Register(const std::string& keys, std::function<T(bool, T)> validator) {
        using namespace config_detail;

        static_assert(is_string_like<T>::value || !std::is_pointer_v<T>,
                      "Raw pointers are not allowed in Config. Use std::shared_ptr"
                      "or other copyable objects instead.");

        std::lock_guard<std::mutex> lock(_mutex);
        using NormalT = normalize_type<T>;
        std::vector<std::string> key_parts = str::Split(keys, '.');
        int level_size = static_cast<int>(key_parts.size());

        for (int i = 0; i < level_size; ++i) {
            const auto& present_key = key_parts[i];
            if (present_key.empty() || (present_key.find(' ') != std::string::npos) ||
                present_key.find("__") == 0) {
                auto key_display = present_key.empty() ? "<empty>" : present_key;
                throw std::invalid_argument("Invalid key part: " + key_display);
            }
        }

        auto current_node = _property;
        for (int i = 0; i < level_size - 1; ++i) {
            const auto& present_key = key_parts[i];
            auto& it = (*current_node)[present_key];
            if (it.children == nullptr) {
                it.children = std::make_shared<std::map<std::string, internal_node>>();
            }
            current_node = it.children;
        }


        const auto& last_key = key_parts.back();
        auto& it_last = (*current_node)[last_key];

        if (it_last.children == nullptr) {
            it_last.children = std::make_shared<std::map<std::string, internal_node>>();
        }

        if (it_last.validator) {
            std::cerr << "Warning: Key '" << keys
                      << "' is already registered. Overwriting the validator." << std::endl;
        }

        // 标记非序列化类型
        if constexpr (is_non_serializable<T>::value) {
            it_last.is_non_serializable = true;
        }

        it_last.validator = [validator = std::move(validator)](bool is_set,
                                                               std::any a) -> std::any {
            if (!validator) {
                return a;
            }

            if (!is_set) {
                NormalT default_val = validator(false, T());
                return std::any(default_val);
            }

            // 从 any 中提取归一化后的值
            NormalT v = std::any_cast<NormalT>(a);
            NormalT res = validator(is_set, v);
            return std::any(res);
        };

        it_last.value = it_last.validator(false, T());
    }

    /**
     * @brief 注册配置项，指定默认值
     * @tparam T 配置值的类型
     * @param key 配置项的键，支持 "." 分隔的分层结构
     * @param value 配置项的默认值
     *
     * @note 此方法会将默认值保存，后续 Get 调用若未设置值则返回该默认值
     *
     * @example
     * \code
     * config->Register<int>("video.width", 1920);
     * config->Register<std::string>("app.name", "MyApp");
     * config->Register<std::shared_ptr<Object>>("obj", std::make_shared<Object>());
     * \endcode
     */
    template <typename T>
    void Register(const std::string& key, T value) {
        using namespace config_detail;


        static_assert(is_string_like<T>::value || !std::is_pointer_v<T>,
                      "Raw pointers are not allowed in Config. Use std::shared_ptr,"
                      "or other copyable objects instead.");

        using NormalT = normalize_type<T>;

        auto normalized_value = normalize_value(value);
        return Register<NormalT>(key, [normalized_value](bool is_set, NormalT v) -> NormalT {
            if (!is_set) {
                return normalized_value;
            }
            return v;
        });
    }

    /**
     * @brief 设置配置值，触发验证和回调
     * @tparam T 配置值的类型
     * @param key 配置项的键，必须已通过 Register 注册
     * @param value 要设置的新值
     * @return 可选类型，包含设置前的旧值（仅对序列化类型有效），非序列化类型返回 nullopt
     *
     * @throw std::invalid_argument 如果 key 未注册
     *
     * @note 该方法会：
     *   1. 对新值进行验证和转换（如果指定了验证函数）
     *   2. 更新配置值
     *   3. 触发所有已注册的变更回调函数
     *   4. 对于非序列化类型（函数、共享指针），不返回旧值
     *
     * @example
     * \code
     * config->Register<int>("video.fps", 30);
     * auto old_fps = config->Set<int>("video.fps", 60);  // old_fps = 30
     *
     * auto callback = [](int x) { return x * 2; };
     * config->Register<std::function<int(int)>>("math.double", callback);
     * config->Set<std::function<int(int)>>("math.double", [](int x) { return x * 3; });
     * \endcode
     */
    template <typename T>
    std::optional<T> Set(const std::string& key, T value) {
        using namespace config_detail;

        std::lock_guard<std::mutex> lock(_mutex);
        using NormalT = normalize_type<T>;
        auto normalized = normalize_value(value);
        auto result = std::optional<T>{};

        std::vector<std::string> key_parts = str::Split(key, '.');
        auto current_node = _property;
        for (size_t i = 0; i < key_parts.size(); ++i) {
            if (current_node == nullptr) {
                throw std::invalid_argument("Key not registered: " + key);
            }

            const auto& present_key = key_parts[i];
            if (!current_node->contains(present_key)) {
                throw std::invalid_argument("Key not registered: " + present_key + " in " + key);
            }

            if (i == key_parts.size() - 1) {
                auto& valid_func = (*current_node)[present_key].validator;

                NormalT final_value = normalized;
                if (valid_func) {
                    std::any input = normalized;
                    std::any res = valid_func(true, input);
                    final_value = std::any_cast<NormalT>(res);
                }

                // 仅对序列化类型处理旧值的返回
                if ((*current_node)[present_key].value.has_value() &&
                    !(*current_node)[present_key].is_non_serializable) {
                    auto t = std::any_cast<NormalT>((*current_node)[present_key].value);
                    // 如果是字符判断转换返回值
                    if constexpr (std::is_same_v<T, std::string>) {
                        result = t;
                    } else if constexpr (std::is_same_v<T, const char*>) {
                        result = t.c_str();
                    } else if constexpr (std::is_same_v<T, char*>) {
                        result = const_cast<char*>(t.c_str());
                    } else if constexpr (std::is_same_v<T, std::string_view>) {
                        result = std::string_view(t);
                    } else if constexpr (std::is_enum_v<T>) {
                        result = static_cast<T>(t);
                    } else {
                        result = t;
                    }
                }

                (*current_node)[present_key].value = final_value;

                for (const auto& callback : (*current_node)[present_key].change_callbacks) {
                    callback(final_value);
                }

                return result;
            }

            current_node = (*current_node)[present_key].children;
        }
        return result;
    }

    /**
     * @brief 获取配置值
     * @tparam T 配置值的类型，必须与注册时的类型匹配
     * @param key 配置项的键
     * @return 可选类型，包含配置值（如果已设置）或默认值（如果通过验证函数定义）
     *         如果键未注册或无值且无默认值，返回 std::nullopt
     *
     * @throw std::bad_any_cast 如果请求的类型与注册时的类型不匹配
     *
     * @note 该方法会：
     *   1. 查找指定键的配置值
     *   2. 如果值不存在但有验证函数，调用验证函数获取默认值
     *   3. 对类型进行自动转换（如 char* 到 std::string）
     *   4. 线程安全，与 Set 操作互斥
     *
     * @example
     * \code
     * config->Register<int>("video.fps", 30);
     * auto fps = config->Get<int>("video.fps");  // 返回 optional<int>，值为 30
     * if (fps) {
     *     std::cout << "FPS: " << fps.value() << std::endl;
     * }
     *
     * auto func = config->Get<std::function<int(int)>>("math.double");
     * if (func) {
     *     int result = func.value()(5);  // 调用获取到的函数
     * }
     * \endcode
     */
    template <typename T>
    std::optional<T> Get(const std::string& key) {
        using namespace config_detail;

        std::lock_guard<std::mutex> lock(_mutex);
        using NormalT = normalize_type<T>;

        std::vector<std::string> key_parts = str::Split(key, '.');
        auto current_node = _property;
        for (size_t i = 0; i < key_parts.size(); ++i) {
            const auto& present_key = key_parts[i];
            if (!current_node->contains(present_key)) {
                return std::nullopt;
            }

            if (i == key_parts.size() - 1) {
                auto& node = (*current_node)[present_key];
                if (!node.value.has_value()) {
                    if (node.validator) {
                        std::any default_any = node.validator(false, std::any());
                        NormalT default_val = std::any_cast<NormalT>(default_any);
                        // 如果 T 是 enum，从 int 转换回 enum
                        if constexpr (std::is_enum_v<T>) {
                            return static_cast<T>(default_val);
                        } else {
                            return default_val;
                        }
                    }
                    return std::nullopt;
                }

                try {
                    NormalT val = std::any_cast<NormalT>(node.value);
                    // 如果 T 是 enum，从 int 转换回 enum
                    if constexpr (std::is_enum_v<T>) {
                        return static_cast<T>(val);
                    } else {
                        return val;
                    }
                } catch (const std::bad_any_cast& e) {
                    std::string err_msg =
                        "Type mismatch when getting key '" + key + "': " + e.what();
                    throw std::runtime_error(err_msg);
                }
            }

            current_node = (*current_node)[present_key].children;
            if (current_node == nullptr) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }


    /**
     * @brief 检查指定的配置键是否已注册
     * @param key 配置项的键
     * @return true 如果键已注册，false 否则
     *
     * @note 该方法只检查键是否存在，不检查是否设置了值
     */
    bool Has(const std::string& key) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> key_parts = str::Split(key, '.');
        auto current_node = _property;
        for (size_t i = 0; i < key_parts.size(); ++i) {
            const auto& present_key = key_parts[i];
            if (!current_node->contains(present_key)) {
                return false;
            }

            if (i == key_parts.size() - 1) {
                return true;
            }

            current_node = (*current_node)[present_key].children;
            if (current_node == nullptr) {
                return false;
            }
        }
        return false;
    }

    /**
     * @brief 注册配置变更回调函数
     * @param key 配置项的键，必须已通过 Register 注册
     * @param callback 变更时的回调函数，接收 std::any 类型的新值
     *
     * @throw std::invalid_argument 如果 key 未注册
     *
     * @note 该方法会：
     *   1. 在指定键的配置值变更时调用 callback
     *   2. 可以注册多个回调函数，按注册顺序执行
     *   3. 回调函数在 Set 之前就已注册则会被触发
     *   4. 线程安全，与 Set 操作互斥
     *
     * @example
     * \code
     * config->Register<int>("video.fps", 30);
     * config->NotifyChange("video.fps", [](std::any new_value) {
     *     int fps = std::any_cast<int>(new_value);
     *     std::cout << "FPS changed to: " << fps << std::endl;
     * });
     * config->Set<int>("video.fps", 60);  // 触发上面的回调
     * \endcode
     */
    void NotifyChange(const std::string& key, std::function<void(std::any)> callback) {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> key_parts = str::Split(key, '.');
        auto current_node = _property;
        for (size_t i = 0; i < key_parts.size(); ++i) {
            const auto& present_key = key_parts[i];
            if (!current_node->contains(present_key)) {
                throw std::invalid_argument("Key not registered: " + present_key);
            }

            if (i == key_parts.size() - 1) {
                (*current_node)[present_key].change_callbacks.push_back(callback);
                return;
            }

            current_node = (*current_node)[present_key].children;
            if (current_node == nullptr) {
                throw std::invalid_argument("Key not registered: " + present_key);
            }
        }
    }


#ifdef BRE_CONFIG_JSON_SUPPORT
    /**
     * @brief 将所有配置序列化为 JSON 字符串
     * @param indent 是否格式化输出（添加缩进和换行），默认为 false
     * @return JSON 格式的字符串表示
     *
     * @note 该方法会：
     *   1. 递归遍历所有配置键值
     *   2. 跳过非序列化类型（函数、共享指针）
     *   3. 非序列化类型以 "__non_serializable": "this is a non-serializable object" 标记
     *   4. 支持嵌套结构，层级由 "." 分隔符决定
     *   5. 空值字段显示为 JSON null
     */
    std::string ToJson(bool indent = false) const noexcept {
        std::lock_guard<std::mutex> lock(_mutex);
        bre::json::Value json_value;
        std::function<void(const std::map<std::string, internal_node>&, bre::json::Value&)>
            build_json = [&](const std::map<std::string, internal_node>& node_map,
                             bre::json::Value& json) {
                std::vector<std::string> keys;
                for (const auto& [key, _] : node_map) {
                    keys.push_back(key);
                }
                std::sort(keys.begin(), keys.end());
                for (const auto& key : keys) {
                    const auto& node = node_map.at(key);

                    if (node.children && !node.children->empty()) {
                        bre::json::Value child_json;
                        child_json.SetObject();
                        build_json(*node.children, child_json);

                        if (node.is_non_serializable) {
                            child_json["__non_serializable"] = "this is a non-serializable object";
                        } else {
                            if (node.value.has_value()) {
                                if (node.value.type() == typeid(std::string)) {
                                    child_json["__value"] = std::any_cast<std::string>(node.value);
                                } else if (node.value.type() == typeid(int)) {
                                    child_json["__value"] = std::any_cast<int>(node.value);
                                } else if (node.value.type() == typeid(bool)) {
                                    child_json["__value"] = std::any_cast<bool>(node.value);
                                } else if (node.value.type() == typeid(double)) {
                                    child_json["__value"].SetDouble(
                                        std::any_cast<double>(node.value));
                                } else {
                                    child_json["__value"] = "Unsupported type: " +
                                                            std::string(node.value.type().name());
                                }
                            }
                        }

                        json[key] = child_json;
                    } else {
                        if (node.is_non_serializable) {
                            bre::json::Value v;
                            v.SetObject();
                            v["__non_serializable"] = "this is a non-serializable object";
                            json[key] = v;
                        } else if (!node.value.has_value()) {
                            json[key] = bre::json::Value();  // set to null if no value
                        } else {
                            if (node.value.type() == typeid(std::string)) {
                                json[key] = std::any_cast<std::string>(node.value);
                            } else if (node.value.type() == typeid(int)) {
                                json[key] = std::any_cast<int>(node.value);
                            } else if (node.value.type() == typeid(bool)) {
                                bre::json::Value v;
                                v.SetBool(std::any_cast<bool>(node.value));
                                json[key] = v;
                            } else if (node.value.type() == typeid(double)) {
                                json[key] = std::any_cast<double>(node.value);
                            } else {
                                json[key] = "Unsupported enum type: " +
                                            std::string(node.value.type().name());
                            }
                        }
                    }
                }
            };
        build_json(*_property, json_value);
        return json_value.ToString(indent, 2, true);
    }

    /**
     * @brief 从 JSON 字符串反序列化配置
     * @param json_str JSON 格式的字符串
     *
     * @throw std::invalid_argument 如果 JSON 格式无效或值类型不支持
     *
     * @note 该方法会：
     *   1. 解析 JSON 字符串
     *   2. 将 JSON 值映射到已注册的配置键
     *   3. 跳过未注册的键
     *   4. 跳过非序列化类型的反序列化（保留原值）
     *   5. 对每个值调用验证函数进行转换
     *   6. 支持嵌套结构与 ToJson 对应
     */
    void FromJson(const std::string& json_str) {
        std::lock_guard<std::mutex> lock(_mutex);

        bre::json::Value json_value;
        try {
            json_value = bre::json::Parser::parse(json_str);
        } catch (const std::exception& e) {
            throw std::invalid_argument("FromJson Invalid JSON string: " + std::string(e.what()));
        }

        std::function<void(const bre::json::Value&, std::map<std::string, internal_node>&)>
            build_config = [&](const bre::json::Value& json,
                               std::map<std::string, internal_node>& node_map) {
                for (const auto& [key, value] : json.AsObject()) {
                    auto& node = node_map[key];

                    // 跳过非序列化类型的反序列化
                    if (node.is_non_serializable) {
                        continue;
                    }

                    if (value.IsObject()) {
                        if (node.children == nullptr) {
                            node.children =
                                std::make_shared<std::map<std::string, internal_node>>();
                        }

                        // 如果对象中有 __value 字段，先设置值
                        if (value.AsObject().count("__value") > 0) {
                            const auto& val = value.AsObject().at("__value");
                            if (val.IsString()) {
                                node.value = val.AsString();
                            } else if (val.IsInt()) {
                                node.value = val.AsInt();
                            } else if (val.IsBool()) {
                                node.value = val.AsBool();
                            } else if (val.IsDouble()) {
                                node.value = val.AsDouble();
                            }
                        }

                        build_config(value, *node.children);
                    } else {
                        std::any input;
                        if (value.IsString()) {
                            input = value.AsString();
                        } else if (value.IsInt()) {
                            input = value.AsInt();
                        } else if (value.IsBool()) {
                            input = value.AsBool();
                        } else if (value.IsDouble()) {
                            input = value.AsDouble();
                        } else {
                            throw std::invalid_argument("Unsupported JSON value type for key: " +
                                                        key);
                        }
                        if (node.validator) {
                            std::any res = node.validator(true, input);
                            node.value = res;
                        } else {
                            node.value = input;
                        }
                    }
                }
            };
        build_config(json_value, *_property);
    }

#endif  // BRE_CONFIG_JSON_SUPPORT

    /**
     * @brief 获取所有已注册的配置键
     * @return 包含所有配置键的向量，键使用 "." 分隔符表示层级关系
     *
     * @note 该方法返回的键按字母顺序排列
     *
     * @example
     * \code
     * config->Register<int>("video.width", 1920);
     * config->Register<int>("video.fps", 30);
     * config->Register<std::string>("app.name", "MyApp");
     * auto keys = config->GetAllKeys();
     * // 返回: ["app.name", "video.fps", "video.width"]
     * \endcode
     */
    std::vector<std::string> GetAllKeys() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> keys;
        std::function<void(const std::map<std::string, internal_node>&, const std::string&)>
            collect_keys = [&](const std::map<std::string, internal_node>& node_map,
                               const std::string& prefix) {
                for (const auto& [key, node] : node_map) {
                    std::string full_key = prefix.empty() ? key : prefix + "." + key;
                    keys.push_back(full_key);
                    if (node.children && !node.children->empty()) {
                        collect_keys(*node.children, full_key);
                    }
                }
            };
        collect_keys(*_property, "");
        return keys;
    }

    /**
     * @brief 重置所有配置值为默认值
     *
     * @note 该方法会：
     *   1. 调用每个配置项的验证函数获取默认值（is_set=false）
     *   2. 恢复所有值为注册时指定的默认值
     *   3. 不触发变更回调
     *   4. 线程安全
     *
     * @example
     * \code
     * config->Register<int>("video.fps", 30);
     * config->Set<int>("video.fps", 60);
     * config->Reset();  // fps 恢复到 30
     * auto fps = config->Get<int>("video.fps");  // 返回 30
     * \endcode
     */
    void Reset() {
        std::lock_guard<std::mutex> lock(_mutex);
        std::function<void(std::map<std::string, internal_node>&)> to_default =
            [&](std::map<std::string, internal_node>& node_map) {
                for (auto& [key, node] : node_map) {
                    if (node.children && !node.children->empty()) {
                        to_default(*node.children);
                    }
                    if (node.validator) {
                        // 因为 validator 第二个参数需要 T, 但是我们无法直接获取 T 的类型，
                        // validator 在第一个参数是false时不会使用第二个参数
                        std::any default_any = node.validator(false, std::any());
                        node.value = default_any;
                    } else {
                        node.value.reset();
                    }
                }
            };
        to_default(*_property);
    }


private:
    Config() : _property(std::make_shared<std::map<std::string, internal_node>>()) {}


    struct internal_node {
        std::shared_ptr<std::map<std::string, internal_node>> children;
        std::function<std::any(bool, std::any)> validator;
        std::vector<std::function<void(std::any)>> change_callbacks;
        std::any value;
        bool is_non_serializable = false;  // 标记是否为非序列化类型（共享指针、函数等）
    };


private:
    std::shared_ptr<std::map<std::string, internal_node>> _property;
    mutable std::mutex _mutex;
};

}  // namespace bre
