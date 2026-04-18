#include "Player.h"

Player::Player(int id, const std::string& name)
    : id_(id), name_(name), gold_(kInitGold), position_(0) {}

bool Player::has_alive_monsters() const {
    for (const auto& m : monsters_) {
        if (m.is_alive()) return true;
    }
    return false;
}

void Player::add_monster(const Monster& m) {
    if (monsters_.size() < kMaxMonsters) {
        monsters_.push_back(m);
    }
}

bool Player::remove_monster(int index) {
    if (index < 0 || index >= static_cast<int>(monsters_.size())) return false;
    monsters_.erase(monsters_.begin() + index);
    return true;
}

bool Player::remove_property(int tile_index) {
    for (auto it = properties_.begin(); it != properties_.end(); ++it) {
        if (*it == tile_index) {
            properties_.erase(it);
            return true;
        }
    }
    return false;
}

void Player::add_item(const Item& item) {
    // 如果同类道具已存在，累加数量
    for (auto& existing : inventory_) {
        if (existing.type == item.type) {
            existing.count += item.count;
            return;
        }
    }
    inventory_.push_back(item);
}

bool Player::use_item(int index) {
    if (index < 0 || index >= static_cast<int>(inventory_.size())) return false;
    auto& item = inventory_[index];

    // 伤药/万能药：治疗第一只存活的精灵
    if (item.type == ItemType::Heal50 || item.type == ItemType::Heal100 ||
        item.type == ItemType::Antidote) {
        for (auto& mon : monsters_) {
            if (mon.is_alive()) {
                mon.heal(item.heal_amount());
                --item.count;
                if (item.count <= 0) inventory_.erase(inventory_.begin() + index);
                return true;
            }
        }
        return false;
    }

    // 复活草：复活第一只濒死的精灵到50%HP
    if (item.type == ItemType::Revive) {
        for (auto& mon : monsters_) {
            if (!mon.is_alive()) {
                mon.heal(mon.max_hp() / 2);
                --item.count;
                if (item.count <= 0) inventory_.erase(inventory_.begin() + index);
                return true;
            }
        }
        return false;
    }

    // 精灵球/超级球：此处不可用（在战斗中使用）
    if (item.type == ItemType::Pokeball || item.type == ItemType::SuperBall) {
        return false;
    }

    // 进化石：此处不可用（在特殊界面使用）
    return false;
}

bool Player::is_bankrupt() const {
    if (gold_ > 0) return false;
    return total_properties() == 0;
}

int Player::total_assets() const {
    int value = gold_;
    // 地皮按购买价计算（简化版）
    return value;
}
