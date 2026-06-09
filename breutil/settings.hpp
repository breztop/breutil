#pragma once
#include <mutex>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include "ini.hpp"

namespace bre {

class Settings {
private:
    Ini& _file_config;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> _default_config;
    std::string _current_section;
    std::string _config_path;
    mutable std::mutex _mutex;

public:
    explicit Settings(std::string config_name)
        : _file_config(Ini::Instance(config_name))
        , _config_path(std::move(config_name))
        , _current_section("default") {}

    ~Settings() = default;


    void UseSection(const std::string& section) {
        std::lock_guard<std::mutex> lock(_mutex);
        _current_section = section;
    }


    std::string GetStr(const std::string& key, const std::string& def = "") {
        return GetStr(_current_section, key, def);
    }

    int GetInt(const std::string& key, int def = 0) { return GetInt(_current_section, key, def); }

    double GetFloat(const std::string& key, double def = 0.0) {
        return GetDouble(_current_section, key, def);
    }

    bool GetBool(const std::string& key, bool def = false) {
        return GetBool(_current_section, key, def);
    }


    std::string GetStr(const std::string& section, const std::string& key,
                       const std::string& def = "") {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_file_config.HasKey(section, key)) return _file_config.GetStr(section, key);

        auto s = _default_config.find(section);
        if (s != _default_config.end()) {
            auto k = s->second.find(key);
            if (k != s->second.end()) return k->second;
        }
        return def;
    }

    int GetInt(const std::string& section, const std::string& key, int def = 0) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_file_config.HasKey(section, key)) return _file_config.GetInt(section, key);

        auto s = _default_config.find(section);
        if (s != _default_config.end()) {
            auto k = s->second.find(key);
            if (k != s->second.end()) return std::stoi(k->second);
        }
        return def;
    }

    double GetDouble(const std::string& section, const std::string& key, double def = 0.0) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_file_config.HasKey(section, key)) return _file_config.GetDouble(section, key);

        auto s = _default_config.find(section);
        if (s != _default_config.end()) {
            auto k = s->second.find(key);
            if (k != s->second.end()) return std::stod(k->second);
        }
        return def;
    }

    bool GetBool(const std::string& section, const std::string& key, bool def = false) {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_file_config.HasKey(section, key)) return _file_config.GetBool(section, key);

        auto s = _default_config.find(section);
        if (s != _default_config.end()) {
            auto k = s->second.find(key);
            if (k != s->second.end()) return k->second == "true" || k->second == "1";
        }
        return def;
    }

    void SetDefault(const std::string& key, const std::string& value) {
        SetDefault(_current_section, key, value);
    }

    void SetDefault(const std::string& key, int value) {
        SetDefault(_current_section, key, std::to_string(value));
    }

    void SetDefault(const std::string& key, bool value) {
        SetDefault(_current_section, key, value ? "true" : "false");
    }

    void SetDefault(const std::string& section, const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _default_config[section][key] = value;
    }


    void Set(const std::string& key, const std::string& value) {
        Set(_current_section, key, value);
    }

    void Set(const std::string& key, int value) {
        Set(_current_section, key, std::to_string(value));
    }

    void Set(const std::string& key, bool value) {
        Set(_current_section, key, value ? "true" : "false");
    }

    void Set(const std::string& section, const std::string& key, const std::string& value) {
        std::lock_guard<std::mutex> lock(_mutex);
        _file_config.Set(section, key, value);
    }

    bool HasKey(const std::string& key) { return HasKey(_current_section, key); }
    bool HasKey(const std::string& section, const std::string& key) {
        std::lock_guard<std::mutex> lock(_mutex);
        return _file_config.HasKey(section, key) ||
               (_default_config.count(section) && _default_config[section].count(key));
    }

    bool Save() { return _file_config.Save(); }
    bool Reload() { return _file_config.Reload(_config_path); }

    std::vector<std::string> AllSections() { return _file_config.GetAllSections(); }
    std::vector<std::string> AllKeys() { return AllKeys(_current_section); }
    std::vector<std::string> AllKeys(const std::string& section) {
        return _file_config.GetAllKeys(section);
    }

    bool RemoveSection(const std::string& section) { return _file_config.RemoveSection(section); }
    bool RemoveKey(const std::string& key) { return RemoveKey(_current_section, key); }
    bool RemoveKey(const std::string& section, const std::string& key) {
        return _file_config.RemoveKey(section, key);
    }
};

}  // namespace bre