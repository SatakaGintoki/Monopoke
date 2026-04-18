#include "EconomySystem.h"

EconomySystem& EconomySystem::get() {
    static EconomySystem instance;
    return instance;
}

bool EconomySystem::try_purchase(Player& player, Tile& tile) {
    if (tile.has_owner()) return false;
    if (tile.type() != TileType::Empty) return false;
    if (player.gold() < tile.price()) return false;

    if (player.spend_gold(tile.price())) {
        tile.set_owner(player.id());
        player.add_property(tile.index());
        return true;
    }
    return false;
}

bool EconomySystem::try_upgrade(Player& player, Tile& tile) {
    if (!tile.can_upgrade()) return false;
    if (tile.owner_id() != player.id()) return false;

    int cost = tile.upgrade_cost();
    if (player.spend_gold(cost)) {
        tile.upgrade();
        return true;
    }
    return false;
}

bool EconomySystem::pay_toll(Player& from, Player& to, Tile& tile) {
    if (!tile.has_owner()) return false;
    if (tile.owner_id() == from.id()) return false;

    int toll = tile.toll();
    if (from.gold() < toll) {
        // 金币不足，尝试卖掉地皮
        return false;
    }

    if (from.spend_gold(toll)) {
        to.add_gold(toll);
        return true;
    }
    return false;
}

int EconomySystem::calculate_total_assets(const Player& player, const std::vector<Tile>& tiles) const {
    int total = player.gold();
    for (int prop_idx : player.properties()) {
        if (prop_idx >= 0 && prop_idx < static_cast<int>(tiles.size())) {
            const Tile& t = tiles[prop_idx];
            if (t.has_owner() && t.owner_id() == player.id()) {
                // 地皮价值 = 已支付价格的80%
                int base_price = t.price();
                int level_bonus = 0;
                for (int i = 1; i <= static_cast<int>(t.level()); ++i) {
                    level_bonus += kPropertyBasePrices[i];
                }
                total += static_cast<int>((base_price + level_bonus) * 0.8f);
            }
        }
    }
    return total;
}

bool EconomySystem::is_bankrupt(const Player& player, const std::vector<Tile>& tiles) const {
    if (player.gold() > 0) return false;
    // 金币为0且没有任何值钱资产才是破产
    return calculate_total_assets(player, tiles) == 0;
}

int EconomySystem::get_property_value(const Tile& tile) const {
    int base_price = tile.price();
    int level_bonus = 0;
    for (int i = 1; i <= static_cast<int>(tile.level()); ++i) {
        level_bonus += kPropertyBasePrices[i];
    }
    return static_cast<int>((base_price + level_bonus) * 0.8f);
}
