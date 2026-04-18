#pragma once

#include "Item.h"
#include "Monster.h"
#include <vector>
#include <string>

class Player {
public:
    Player() = default;
    Player(int id, const std::string& name);

    // 基础属性
    int id() const { return id_; }
    const std::string& name() const { return name_; }

    // 金币
    int gold() const { return gold_; }
    void add_gold(int amount) { gold_ += amount; }
    bool spend_gold(int amount) {
        if (gold_ < amount) return false;
        gold_ -= amount;
        return true;
    }

    // 位置
    int position() const { return position_; }
    void set_position(int pos) { position_ = pos; }

    // 已走圈数
    int laps_completed() const { return laps_completed_; }
    void add_lap() { ++laps_completed_; }

    // 精灵管理
    const std::vector<Monster>& monsters() const { return monsters_; }
    std::vector<Monster>& monsters() { return monsters_; }
    bool has_alive_monsters() const;
    void add_monster(const Monster& m);
    bool remove_monster(int index);

    // 地皮管理
    const std::vector<int>& properties() const { return properties_; }
    void add_property(int tile_index) { properties_.push_back(tile_index); }
    bool remove_property(int tile_index);
    int total_properties() const { return static_cast<int>(properties_.size()); }

    // 道具管理
    const std::vector<Item>& inventory() const { return inventory_; }
    std::vector<Item>& inventory() { return inventory_; }
    void add_item(const Item& item);
    bool use_item(int index);

    // 破产判定
    bool is_bankrupt() const;
    int total_assets() const;  // 金币 + 所有地皮价值

private:
    int id_ = 0;
    std::string name_;
    int gold_ = kInitGold;
    int position_ = 0;
    int laps_completed_ = 0;
    std::vector<Monster> monsters_;
    std::vector<int> properties_;     // 拥有的地皮索引
    std::vector<Item> inventory_;
};
