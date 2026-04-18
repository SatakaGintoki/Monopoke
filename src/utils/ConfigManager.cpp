#include "ConfigManager.h"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

ConfigManager& ConfigManager::get() {
    static ConfigManager instance;
    return instance;
}

std::string ConfigManager::get_data_path() const {
    return "assets/data/";
}

json ConfigManager::load_json(const std::string& filename) {
    std::ifstream f(get_data_path() + filename);
    if (!f.is_open()) {
        std::cerr << "[ConfigManager] Failed to open: " << filename << std::endl;
        return json::object();
    }
    try {
        return json::parse(f);
    } catch (const json::parse_error& e) {
        std::cerr << "[ConfigManager] Parse error in " << filename << ": " << e.what() << std::endl;
        return json::array();
    }
}

void ConfigManager::load_all() {
    monsters_ = load_json("monsters.json");
    skills_ = load_json("skills.json");
    items_ = load_json("items.json");

    if (!monsters_.empty()) {
        std::cout << "[ConfigManager] Loaded " << monsters_.size() << " monster templates" << std::endl;
    }
    if (!skills_.empty()) {
        std::cout << "[ConfigManager] Loaded " << skills_.size() << " skill templates" << std::endl;
    }
    if (!items_.empty()) {
        std::cout << "[ConfigManager] Loaded " << items_.size() << " item templates" << std::endl;
    }
}

const json* ConfigManager::monster_template(int id) const {
    for (const auto& m : monsters_) {
        if (m.value("id", -1) == id) return &m;
    }
    return nullptr;
}

const json* ConfigManager::skill_template(int id) const {
    for (const auto& s : skills_) {
        if (s.value("id", -1) == id) return &s;
    }
    return nullptr;
}
