#include "SaveManager.h"
#include "../entity/Player.h"
#include "../entity/Board.h"
#include "../entity/Monster.h"
#include "../entity/Item.h"
#include "../system/MonsterSystem.h"
#include "../utils/Types.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

SaveManager& SaveManager::get() {
    static SaveManager instance;
    return instance;
}

namespace {

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

Monster deserialize_monster(const nlohmann::json& j) {
    int tid = j.value("id", 1);
    int lvl = j.value("level", 1);
    Monster mon = MonsterSystem::get().create_monster(tid, lvl);
    if (mon.id() == 0) {
        mon = MonsterSystem::get().create_monster(1, std::max(1, lvl));
    }

    mon.set_max_hp(j.value("max_hp", mon.max_hp()));
    int chp = j.value("current_hp", mon.current_hp());
    mon.set_current_hp(std::min(chp, mon.max_hp()));
    mon.set_atk(j.value("atk", mon.atk()));
    mon.set_def(j.value("def", mon.def()));
    mon.set_spd(j.value("spd", mon.spd()));
    mon.set_sp_atk(j.value("sp_atk", mon.sp_atk()));
    mon.set_sp_def(j.value("sp_def", mon.sp_def()));
    mon.set_exp(j.value("exp", mon.exp()));
    mon.set_exp_to_next(j.value("exp_to_next", mon.exp_to_next()));
    mon.set_rarity(static_cast<Rarity>(j.value("rarity", static_cast<int>(mon.rarity()))));

    if (j.contains("skills") && j["skills"].is_array() && !j["skills"].empty()) {
        mon.clear_skills();
        for (const auto& sj : j["skills"]) {
            Skill s;
            s.name = sj.value("name", std::string("撞击"));
            s.type = static_cast<MonsterType>(sj.value("type", 0));
            s.power = sj.value("power", 40);
            s.max_pp = sj.value("max_pp", sj.value("pp", 35));
            s.pp = sj.value("pp", s.max_pp);
            s.accuracy = sj.value("accuracy", 1.0f);
            s.category = static_cast<SkillCategory>(sj.value("category", 0));
            mon.add_skill(s);
        }
    }
    return mon;
}

nlohmann::json serialize_player(const Player& player) {
    nlohmann::json jo;
    jo["id"] = player.id();
    jo["name"] = player.name();
    jo["gold"] = player.gold();
    jo["position"] = player.position();
    jo["laps_completed"] = player.laps_completed();

    nlohmann::json monsters_json = nlohmann::json::array();
    for (const auto& mon : player.monsters()) {
        monsters_json.push_back(serialize_monster(mon));
    }
    jo["monsters"] = monsters_json;

    nlohmann::json props_json = nlohmann::json::array();
    for (int prop : player.properties()) {
        props_json.push_back(prop);
    }
    jo["properties"] = props_json;

    nlohmann::json inv = nlohmann::json::array();
    for (const auto& item : player.inventory()) {
        nlohmann::json ij;
        ij["type"] = static_cast<int>(item.type);
        ij["name"] = item.name;
        ij["price"] = item.price;
        ij["count"] = item.count;
        inv.push_back(ij);
    }
    jo["inventory"] = inv;
    return jo;
}

void deserialize_player(Player& p, const nlohmann::json& j) {
    p.set_id(j.value("id", kPlayer1Id));
    p.set_name(j.value("name", std::string("Player")));
    p.set_gold(j.value("gold", kInitGold));
    p.set_position(j.value("position", 0));
    p.set_laps_completed(j.value("laps_completed", 0));

    p.monsters().clear();
    for (const auto& mj : j.value("monsters", nlohmann::json::array())) {
        if (!mj.is_object()) {
            continue;
        }
        p.add_monster(deserialize_monster(mj));
    }

    p.clear_properties();
    for (const auto& pr : j.value("properties", nlohmann::json::array())) {
        if (pr.is_number()) {
            p.add_property(pr.get<int>());
        }
    }

    p.clear_inventory();
    for (const auto& ij : j.value("inventory", nlohmann::json::array())) {
        if (!ij.is_object()) {
            continue;
        }
        Item it;
        it.type = static_cast<ItemType>(ij.value("type", 0));
        it.name = ij.value("name", std::string(""));
        it.price = ij.value("price", 0);
        it.count = ij.value("count", 1);
        p.add_item(it);
    }
}

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
        tile_j["guardian_idx"] = tile.guardian_idx();
        tiles_json.push_back(tile_j);
    }
    j["tiles"] = tiles_json;
    return j;
}

} // namespace

bool SaveManager::save_play_state(const Player& p1, const Player& p2, const Board& board,
                                  int current_player, int turn_count, int game_mode,
                                  const std::string& path) {
    try {
        std::filesystem::path p(path);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (const std::exception& e) {
        std::cerr << "[SaveManager] create_directories: " << e.what() << std::endl;
    }

    nlohmann::json root;
    root["version"] = 1;
    root["player1"] = serialize_player(p1);
    root["player2"] = serialize_player(p2);
    root["board"] = serialize_board(board);
    root["current_player"] = current_player;
    root["turn_count"] = turn_count;
    root["game_mode"] = game_mode;
    root["save_info"] = "Auto Save";

    std::ofstream out(path);
    if (!out) {
        std::cerr << "[SaveManager] Failed to open for write: " << path << std::endl;
        return false;
    }
    out << root.dump(2);
    last_save_path_ = path;
    return true;
}

bool SaveManager::load_play_state(Player& p1, Player& p2, Board& board, int& current_player,
                                  int& turn_count, int& game_mode, const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }

    try {
        nlohmann::json j;
        in >> j;
        if (!j.contains("player1") || !j.contains("player2") || !j.contains("board")) {
            return false;
        }
        deserialize_player(p1, j["player1"]);
        deserialize_player(p2, j["player2"]);
        board.restore_from_save(j["board"]);
        current_player = j.value("current_player", 0);
        if (current_player != 0 && current_player != 1) {
            current_player = 0;
        }
        turn_count = j.value("turn_count", 0);
        game_mode = j.value("game_mode", 0);
        last_save_path_ = path;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "[SaveManager] load_play_state: " << e.what() << std::endl;
        return false;
    }
}

bool SaveManager::save_game(const std::string& path) {
    (void)path;
    return false;
}

bool SaveManager::load_game(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    try {
        nlohmann::json j;
        in >> j;
        return j.contains("player1") && j.contains("player2") && j.contains("board");
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
    std::ifstream in(path);
    if (!in) {
        return false;
    }
    try {
        nlohmann::json j;
        in >> j;
        return j.contains("player1") && j.contains("player2") && j.contains("board");
    } catch (...) {
        return false;
    }
}
