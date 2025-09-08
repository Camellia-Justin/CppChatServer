#include "ConfigManager.h"
#include <fstream>
#include <iostream>

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance; 
    return instance;
}

bool ConfigManager::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Config Error: Could not open config file '" << filename << "'." << std::endl;
        isLoaded = false;
        return false;
    }
    try {
        config = json::parse(file);
        isLoaded = true;
        std::cout << "Config file '" << filename << "' loaded successfully." << std::endl;
        return true;
    } catch (const json::parse_error& e) {
        std::cerr << "Config Error: Failed to parse '" << filename << "'.\n"
                  << "Message: " << e.what() << "\n"
                  << "Error ID: " << e.id << " at byte " << e.byte << std::endl;
        isLoaded = false;
        return false;
    }
}

const json& ConfigManager::getConfig() const {
    if (!isLoaded) {
        throw std::runtime_error("Configuration has not been loaded.");
    }
    return config;
}