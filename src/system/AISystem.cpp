#include "AISystem.h"
#include "../entity/Board.h"
#include "../utils/Random.h"
#include "../utils/Types.h"

AISystem& AISystem::get() {
    static AISystem instance;
    return instance;
}

float AISystem::get_type_effectiveness(MonsterType atk_type, MonsterType def_type) const {
    // 简单属性克制表
    static const float table[10][10] = {
        //       Nor  Fir  Wat  Gra  Ele  Ice  Dra  Dar  Gho  Psy
        /*Nor*/ {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
        /*Fire*/{1.0f, 0.5f, 0.5f, 2.0f, 1.0f, 2.0f, 0.5f, 1.0f, 1.0f, 1.0f},
        /*Water*/{1.0f, 2.0f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f},
        /*Grass*/{1.0f, 0.5f, 2.0f, 0.5f, 1.0f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f},
        /*Elec*/{1.0f, 1.0f, 2.0f, 0.5f, 0.5f, 1.0f, 0.5f, 1.0f, 1.0f, 1.0f},
        /*Ice*/ {1.0f, 0.5f, 0.5f, 2.0f, 2.0f, 0.5f, 2.0f, 1.0f, 1.0f, 1.0f},
        /*Dra*/ {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 1.0f, 1.0f},
        /*Dark*/{1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 2.0f},
        /*Gho*/ {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.5f, 2.0f, 1.0f},
        /*Psy*/ {1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 2.0f, 1.0f, 0.5f}
    };

    int atk_idx = static_cast<int>(atk_type);
    int def_idx = static_cast<int>(def_type);
    if (atk_idx >= 0 && atk_idx < 10 && def_idx >= 0 && def_idx < 10) {
        return table[atk_idx][def_idx];
    }
    return 1.0f;
}

int AISystem::choose_skill(const Monster& mon, const Monster& opponent) {
    const auto& skills = mon.skills();
    if (skills.empty()) return -1;

    // 简单AI：随机选择有PP的技能
    // 策略AI：优先选择克制对手的技能，然后选威力最高的

    // 找出所有可用的技能
    std::vector<int> usable_indices;
    for (size_t i = 0; i < skills.size(); ++i) {
        if (skills[i].pp > 0) {
            usable_indices.push_back(static_cast<int>(i));
        }
    }

    if (usable_indices.empty()) return -1;

    // 策略选择：先找克制的技能
    MonsterType opponent_type = opponent.type();
    int best_idx = usable_indices[0];
    float best_score = 0.0f;

    for (int idx : usable_indices) {
        const auto& skill = skills[idx];
        float effectiveness = get_type_effectiveness(skill.type, opponent_type);
        // 评分 = 克制系数 * 威力
        float score = effectiveness * static_cast<float>(skill.power);

        if (score > best_score) {
            best_score = score;
            best_idx = idx;
        }
    }

    // 30%概率随机选择（增加变化）
    if (Random::get().probability(0.3f)) {
        return usable_indices[Random::get().range(0, static_cast<int>(usable_indices.size()) - 1)];
    }

    return best_idx;
}

bool AISystem::should_buy_property(const Player& player, const Tile& tile) const {
    // 金币足够且地皮价格合理（不超过持有金的50%）
    if (player.gold() < tile.price()) return false;
    if (tile.price() > player.gold() * 0.5f) return false;
    if (tile.has_owner()) return false;
    return true;
}

bool AISystem::should_upgrade_property(const Player& player, const Tile& tile) const {
    if (!tile.has_owner() || tile.owner_id() != player.id()) return false;
    if (!tile.can_upgrade()) return false;

    // 升级费用不超过持有金的30%
    int upgrade_cost = tile.upgrade_cost();
    if (upgrade_cost > player.gold() * 0.3f) return false;
    return true;
}

int AISystem::choose_property_to_upgrade(const Player& player, const std::vector<Tile>& tiles) const {
    // 选择最值得升级的地皮（过路费最高的）
    int best_idx = -1;
    int best_toll = 0;

    for (int prop_idx : player.properties()) {
        if (prop_idx >= 0 && prop_idx < static_cast<int>(tiles.size())) {
            const Tile& t = tiles[prop_idx];
            if (t.owner_id() == player.id() && t.can_upgrade()) {
                int toll = t.toll();
                if (toll > best_toll) {
                    best_toll = toll;
                    best_idx = prop_idx;
                }
            }
        }
    }

    return best_idx;
}

std::string AISystem::choose_item_to_buy(const Player& player) const {
    // 检查精灵HP，如果有任何精灵HP低于50%，建议买伤药
    for (const auto& mon : player.monsters()) {
        if (mon.is_alive()) {
            float hp_ratio = static_cast<float>(mon.current_hp()) / static_cast<float>(mon.max_hp());
            if (hp_ratio < 0.5f) {
                return "Heal50";
            }
        }
    }

    // 如果没有精灵或都满血，考虑买精灵球
    if (player.gold() > 800) {
        return "Pokeball";
    }

    return "";
}

int AISystem::choose_guardian_slot(const Player& player, const Board& board) const {
    int best = -1;
    int best_score = -1;
    for (int i = 0; i < static_cast<int>(player.monsters().size()); ++i) {
        const Monster& m = player.monsters()[i];
        if (!m.is_alive()) {
            continue;
        }
        bool used_elsewhere = false;
        for (const Tile& t : board.tiles()) {
            if (t.owner_id() == player.id() && t.has_guardian() && t.guardian_idx() == i) {
                used_elsewhere = true;
                break;
            }
        }
        if (used_elsewhere) {
            continue;
        }
        int score = m.atk() + m.def();
        if (score > best_score) {
            best_score = score;
            best = i;
        }
    }
    return best;
}

bool AISystem::should_use_heal_item(const Player& player) const {
    // 如果有精灵HP低于30%，应该使用道具
    for (const auto& mon : player.monsters()) {
        if (mon.is_alive()) {
            float hp_ratio = static_cast<float>(mon.current_hp()) / static_cast<float>(mon.max_hp());
            if (hp_ratio < 0.3f) {
                return true;
            }
        }
    }
    return false;
}
