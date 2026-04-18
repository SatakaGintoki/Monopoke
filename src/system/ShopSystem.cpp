#include "ShopSystem.h"
#include "../utils/ConfigManager.h"
#include <fstream>

ShopSystem& ShopSystem::get() {
    static ShopSystem instance;
    return instance;
}

void ShopSystem::load_from_json(const std::string& json_path) {
    items_.clear();

    std::ifstream file(json_path);
    if (!file.is_open()) {
        return;
    }

    try {
        nlohmann::json j;
        file >> j;

        for (const auto& item_json : j) {
            Item item;
            std::string type_str = item_json.value("type", "Heal50");
            if (type_str == "Pokeball") item.type = ItemType::Pokeball;
            else if (type_str == "SuperBall") item.type = ItemType::SuperBall;
            else if (type_str == "Heal50") item.type = ItemType::Heal50;
            else if (type_str == "Heal100") item.type = ItemType::Heal100;
            else if (type_str == "Antidote") item.type = ItemType::Antidote;
            else if (type_str == "Revive") item.type = ItemType::Revive;
            else if (type_str == "FireStone") item.type = ItemType::FireStone;
            else if (type_str == "WaterStone") item.type = ItemType::WaterStone;
            else if (type_str == "GrassStone") item.type = ItemType::GrassStone;
            else if (type_str == "ThunderStone") item.type = ItemType::ThunderStone;
            else if (type_str == "IceStone") item.type = ItemType::IceStone;
            else item.type = ItemType::Heal50;

            item.name = item_json.value("name", "");
            item.price = item_json.value("price", 0);
            item.count = 1;

            items_.push_back(item);
        }
    } catch (...) {
        // JSON parse error, return empty
    }
}

bool ShopSystem::buy_item(Player& player, const Item& item_template, int count) {
    int total_cost = item_template.price * count;
    if (player.gold() < total_cost) {
        return false;
    }

    if (!player.spend_gold(total_cost)) {
        return false;
    }

    // 查找背包中是否有同类道具
    for (auto& item : player.inventory()) {
        if (item.type == item_template.type) {
            item.count += count;
            return true;
        }
    }

    // 没有同类道具，添加新的
    Item new_item = item_template;
    new_item.count = count;
    player.add_item(new_item);
    return true;
}

bool ShopSystem::use_item(Player& player, int inventory_index) {
    if (inventory_index < 0 || inventory_index >= static_cast<int>(player.inventory().size())) {
        return false;
    }

    Item& item = player.inventory()[inventory_index];
    if (item.count <= 0) {
        return false;
    }

    bool used = false;

    switch (item.type) {
        case ItemType::Heal50:
        case ItemType::Heal100:
        case ItemType::Antidote: {
            // 治疗精灵
            for (auto& mon : player.monsters()) {
                if (mon.is_alive() && mon.current_hp() < mon.max_hp()) {
                    int heal = (item.type == ItemType::Heal50) ? 50 :
                               (item.type == ItemType::Heal100) ? 100 : 50;
                    mon.heal(heal);
                    used = true;
                    break;
                }
            }
            break;
        }
        case ItemType::Revive: {
            // 复活精灵
            for (auto& mon : player.monsters()) {
                if (!mon.is_alive()) {
                    mon.heal(mon.max_hp() / 2);
                    used = true;
                    break;
                }
            }
            break;
        }
        case ItemType::FireStone:
        case ItemType::WaterStone:
        case ItemType::GrassStone:
        case ItemType::ThunderStone:
        case ItemType::IceStone: {
            // 进化石 - 需要在MonsterSystem中处理
            // 这里只是标记已使用
            used = true;
            break;
        }
        default:
            break;
    }

    if (used) {
        item.count--;
    }

    return used;
}

std::vector<Item> ShopSystem::get_items_by_category(const std::string& category) const {
    std::vector<Item> result;
    for (const auto& item : items_) {
        if (category == "consumable") {
            if (item.type == ItemType::Heal50 || item.type == ItemType::Heal100 ||
                item.type == ItemType::Antidote || item.type == ItemType::Revive ||
                item.type == ItemType::Pokeball || item.type == ItemType::SuperBall) {
                result.push_back(item);
            }
        } else if (category == "evolution") {
            if (item.type == ItemType::FireStone || item.type == ItemType::WaterStone ||
                item.type == ItemType::GrassStone || item.type == ItemType::ThunderStone ||
                item.type == ItemType::IceStone) {
                result.push_back(item);
            }
        }
    }
    return result;
}
