#pragma once

#include "../entity/Player.h"
#include "../entity/Monster.h"
#include "../entity/Tile.h"
#include <vector>
#include <string>

class Board;

class AISystem {
public:
    static AISystem& get();

    // AI决策：选择技能索引
    int choose_skill(const Monster& mon, const Monster& opponent);

    // AI决策：是否购买地皮
    bool should_buy_property(const Player& player, const Tile& tile) const;

    // AI决策：是否升级地皮
    bool should_upgrade_property(const Player& player, const Tile& tile) const;

    // AI决策：选择哪个地皮升级
    int choose_property_to_upgrade(const Player& player, const std::vector<Tile>& tiles) const;

    // AI决策：在商店买什么道具
    std::string choose_item_to_buy(const Player& player) const;

    // AI决策：是否使用道具
    bool should_use_heal_item(const Player& player) const;

    // 选择驻守队伍索引（-1 表示不派驻守）
    int choose_guardian_slot(const Player& player, const Board& board) const;

private:
    AISystem() = default;

    // 获取技能克制系数
    float get_type_effectiveness(MonsterType atk_type, MonsterType def_type) const;
};
