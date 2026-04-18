#pragma once

#include "../utils/Types.h"
#include <string>

struct Item {
    ItemType type = ItemType::Heal50;
    std::string name;
    int price = 0;
    int count = 1;

    int heal_amount() const {
        switch (type) {
            case ItemType::Heal50: return 50;
            case ItemType::Heal100: return 100;
            case ItemType::Antidote: return 50;
            default: return 0;
        }
    }
};
