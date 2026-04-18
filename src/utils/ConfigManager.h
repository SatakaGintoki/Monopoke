#pragma once

#include <string>
#include <vector>
#include "json.hpp"

class ConfigManager {
public:
    static ConfigManager& get();

    // 加载所有数据文件
    void load_all();

    // 精灵模板
    const nlohmann::json& monsters() const { return monsters_; }
    const nlohmann::json* monster_template(int id) const;

    // 技能模板
    const nlohmann::json& skills() const { return skills_; }
    const nlohmann::json* skill_template(int id) const;

    // 道具模板
    const nlohmann::json& items() const { return items_; }

private:
    ConfigManager() = default;

    nlohmann::json monsters_;
    nlohmann::json skills_;
    nlohmann::json items_;

    std::string get_data_path() const;
    nlohmann::json load_json(const std::string& filename);
};
