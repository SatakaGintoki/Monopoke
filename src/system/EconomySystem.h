#pragma once

#include "../entity/Player.h"
#include "../entity/Tile.h"
#include <vector>

class EconomySystem {
public:
    static EconomySystem& get();

    // 尝试购买地皮
    bool try_purchase(Player& player, Tile& tile);

    // 尝试升级地皮
    bool try_upgrade(Player& player, Tile& tile);

    // 收取过路费
    bool pay_toll(Player& from, Player& to, Tile& tile);

    // 计算玩家总资产（金币 + 所有地皮价值）
    int calculate_total_assets(const Player& player, const std::vector<Tile>& tiles) const;

    // 是否破产
    bool is_bankrupt(const Player& player, const std::vector<Tile>& tiles) const;

    // 获取地皮当前价值（购买价的80%）
    int get_property_value(const Tile& tile) const;

private:
    EconomySystem() = default;
};
