#pragma once

#include "../utils/Types.h"

class Tile {
public:
    Tile() = default;
    Tile(int index, TileType type, int price = 0);

    // 基础属性
    int index() const { return index_; }
    TileType type() const { return type_; }
    int price() const { return price_; }

    // 地皮属性
    int owner_id() const { return owner_id_; }
    void set_owner(int id) { owner_id_ = id; }
    bool has_owner() const { return owner_id_ >= 0; }

    PropertyLevel level() const { return level_; }
    void upgrade() { if (level_ != PropertyLevel::Gym) level_ = PropertyLevel(static_cast<int>(level_) + 1); }

    // 驻守精灵（玩家队伍中的索引，-1表示无）
    int guardian_idx() const { return guardian_idx_; }
    void set_guardian(int idx) { guardian_idx_ = idx; }
    void clear_guardian() { guardian_idx_ = -1; }
    bool has_guardian() const { return guardian_idx_ >= 0; }

    // 过路费计算
    int toll() const;

    // 升级费用
    int upgrade_cost() const;

    // 是否可升级
    bool can_upgrade() const { return level_ != PropertyLevel::Gym && has_owner(); }

    // 存档恢复用（不改变格子类型与 index 时也可用）
    void set_index(int idx) { index_ = idx; }
    void set_type(TileType t) { type_ = t; }
    void set_price(int p) { price_ = p; }
    void set_property_level(PropertyLevel l) { level_ = l; }

private:
    int index_ = 0;
    TileType type_ = TileType::Empty;
    int price_ = 0;
    int owner_id_ = -1;           // -1 = 无主
    PropertyLevel level_ = PropertyLevel::Empty;
    int guardian_idx_ = -1;       // 驻守精灵在玩家队伍中的索引
};
