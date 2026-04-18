#pragma once

#include "../entity/Player.h"
#include "../entity/Item.h"
#include <vector>
#include <string>

class ShopSystem {
public:
    static ShopSystem& get();

    // 购买道具
    bool buy_item(Player& player, const Item& item_template, int count = 1);

    // 使用道具（治疗/进化石等）
    bool use_item(Player& player, int inventory_index);

    // 获取商品列表
    const std::vector<Item>& get_items() const { return items_; }

    // 获取商品分类
    std::vector<Item> get_items_by_category(const std::string& category) const;

    // 加载商品数据
    void load_from_json(const std::string& json_path);

private:
    ShopSystem() = default;

    std::vector<Item> items_;
};
