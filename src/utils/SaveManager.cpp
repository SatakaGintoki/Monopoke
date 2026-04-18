#include "SaveManager.h"
#include "../entity/Player.h"
#include "../entity/Board.h"
#include "../entity/Monster.h"
#include "../entity/Tile.h"
#include <fstream>
#include <iostream>

SaveManager& SaveManager::get() {
    static SaveManager instance;
    return instance;
}

bool SaveManager::save_game(const std::string& path) {
    // 保存逻辑由 Game 类调用，这里返回 true 表示使用自动存档
    last_save_path_ = path;
    return true;
}

bool SaveManager::load_game(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        // 加载逻辑由 Game 类调用
        last_save_path_ = path;
        return true;
    } catch (...) {
        return false;
    }
}

std::string SaveManager::get_save_info(const std::string& path) const {
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }

    try {
        nlohmann::json j;
        file >> j;
        return j.value("save_info", "");
    } catch (...) {
        return "";
    }
}

bool SaveManager::has_save(const std::string& path) const {
    std::ifstream file(path);
    return file.is_open();
}

// 全局存档辅助函数
namespace save_helper {

// 序列化精灵
nlohmann::json serialize_monster(const Monster& mon) {
    nlohmann::json j;
    j["id"] = mon.id();
    j["name"] = mon.name();
    j["type"] = static_cast<int>(mon.type());
    j["level"] = mon.level();
    j["current_hp"] = mon.current_hp();
    j["max_hp"] = mon.max_hp();
    j["atk"] = mon.atk();
    j["def"] = mon.def();
    j["spd"] = mon.spd();
    j["sp_atk"] = mon.sp_atk();
    j["sp_def"] = mon.sp_def();
    j["exp"] = mon.exp();
    j["exp_to_next"] = mon.exp_to_next();
    j["rarity"] = static_cast<int>(mon.rarity());
    j["evolve_to_id"] = mon.evolve_to();
    j["evolve_level"] = mon.evolve_level();

    // 技能列表
    nlohmann::json skills_json = nlohmann::json::array();
    for (const auto& skill : mon.skills()) {
        nlohmann::json skill_j;
        skill_j["name"] = skill.name;
        skill_j["type"] = static_cast<int>(skill.type);
        skill_j["power"] = skill.power;
        skill_j["pp"] = skill.pp;
        skill_j["max_pp"] = skill.max_pp;
        skill_j["accuracy"] = skill.accuracy;
        skill_j["category"] = static_cast<int>(skill.category);
        skills_json.push_back(skill_j);
    }
    j["skills"] = skills_json;

    return j;
}

// 反序列化精灵
Monster deserialize_monster(const nlohmann::json& j) {
    Monster mon;

    // 使用MonsterSystem创建精灵会更合适，这里简化处理
    // 实际上需要通过MonsterSystem来正确创建精灵
    return mon;
}

// 序列化玩家
nlohmann::json serialize_player(const Player& player) {
    nlohmann::json j;
    j["id"] = player.id();
    j["name"] = player.name();
    j["gold"] = player.gold();
    j["position"] = player.position();
    j["laps_completed"] = player.laps_completed();

    // 精灵列表
    nlohmann::json monsters_json = nlohmann::json::array();
    for (const auto& mon : player.monsters()) {
        monsters_json.push_back(serialize_monster(mon));
    }
    j["monsters"] = monsters_json;

    // 地皮列表
    nlohmann::json props_json = nlohmann::json::array();
    for (int prop : player.properties()) {
        props_json.push_back(prop);
    }
    j["properties"] = props_json;

    return j;
}

// 序列化棋盘
nlohmann::json serialize_board(const Board& board) {
    nlohmann::json j;
    nlohmann::json tiles_json = nlohmann::json::array();

    for (const auto& tile : board.tiles()) {
        nlohmann::json tile_j;
        tile_j["index"] = tile.index();
        tile_j["type"] = static_cast<int>(tile.type());
        tile_j["price"] = tile.price();
        tile_j["owner_id"] = tile.owner_id();
        tile_j["level"] = static_cast<int>(tile.level());
        tiles_json.push_back(tile_j);
    }
    j["tiles"] = tiles_json;

    return j;
}

} // namespace save_helper

// 公开的序列化接口
nlohmann::json save_game_state(const Player& p1, const Player& p2, const Board& board,
                                int current_player, int turn_count) {
    nlohmann::json j;
    j["version"] = 1;
    j["player1"] = save_helper::serialize_player(p1);
    j["player2"] = save_helper::serialize_player(p2);
    j["board"] = save_helper::serialize_board(board);
    j["current_player"] = current_player;
    j["turn_count"] = turn_count;
    j["save_info"] = "Auto Save";
    return j;
}

bool load_game_state(const std::string& path, Player& p1, Player& p2, Board& board,
                     int& current_player, int& turn_count) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }

    try {
        nlohmann::json j;
        file >> j;
        // 简化实现，实际需要完整的反序列化逻辑
        current_player = j.value("current_player", 0);
        turn_count = j.value("turn_count", 0);
        return true;
    } catch (...) {
        return false;
    }
}
