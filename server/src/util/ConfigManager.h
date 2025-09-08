#pragma once


#include <string>
#include <stdexcept>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class ConfigManager {
public:
    static ConfigManager& getInstance();
    bool load(const std::string& filename);
    const json& getConfig() const;
private:
    json config;
    bool isLoaded = false;

    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
};