#include "EventSystem.h"
#include "../utils/Random.h"
#include <SFML/Graphics.hpp>
#include <fstream>

EventSystem& EventSystem::get() {
    static EventSystem instance;
    return instance;
}

void EventSystem::load_from_json(const std::string& json_path) {
    events_.clear();

    std::ifstream file(json_path);
    if (!file.is_open()) {
        return;
    }

    try {
        nlohmann::json j;
        file >> j;

        for (const auto& e : j) {
            RandomEvent event;
            std::string type_str = e.value("type", "GoodLuck");
            if (type_str == "GoodLuck") event.type = EventType::GoodLuck;
            else if (type_str == "BadLuck") event.type = EventType::BadLuck;
            else if (type_str == "MonsterHurt") event.type = EventType::MonsterHurt;
            else if (type_str == "MonsterHeal") event.type = EventType::MonsterHeal;
            else if (type_str == "FreeCapture") event.type = EventType::FreeCapture;
            else if (type_str == "MonsterLost") event.type = EventType::MonsterLost;
            else event.type = EventType::GoodLuck;

            event.name = e.value("name", "");
            event.description = e.value("description", "");
            event.gold_change = e.value("gold_change", 0);
            event.hp_change = e.value("hp_change", 0);
            event.capture_bonus = e.value("capture_bonus", false);
            event.monster_lost = e.value("monster_lost", false);
            event.probability = e.value("probability", 0.0f);

            events_.push_back(event);
        }
    } catch (...) {
        // JSON parse error
    }
}

RandomEvent EventSystem::trigger_event(Player& player) {
    // 按概率抽取事件
    float roll = Random::get().chance();
    float cumulative = 0.0f;

    for (const auto& event : events_) {
        cumulative += event.probability;
        if (roll <= cumulative) {
            // 应用事件效果
            if (event.gold_change > 0) {
                player.add_gold(event.gold_change);
            } else if (event.gold_change < 0) {
                player.spend_gold(-event.gold_change);
            }

            if (event.hp_change != 0 && !player.monsters().empty()) {
                // 对第一只存活的精灵生效
                for (auto& mon : player.monsters()) {
                    if (mon.is_alive()) {
                        if (event.hp_change < 0) {
                            mon.take_damage(-event.hp_change);
                        } else {
                            mon.heal(event.hp_change);
                        }
                        break;
                    }
                }
            }

            if (event.monster_lost && player.monsters().size() > 1) {
                // 随机失去一只精灵（除了第一只）
                int lost_idx = 1 + Random::get().range(0, static_cast<int>(player.monsters().size() - 1));
                player.remove_monster(lost_idx);
            }

            return event;
        }
    }

    // 默认返回第一个事件
    return events_.empty() ? RandomEvent{} : events_[0];
}

std::wstring EventSystem::get_event_description(const RandomEvent& event) const {
    std::wstring desc = L"";
    desc += sf::String(event.description).toWideString();

    if (event.gold_change > 0) {
        desc += L" +" + std::to_wstring(event.gold_change) + L"G";
    } else if (event.gold_change < 0) {
        desc += L" " + std::to_wstring(event.gold_change) + L"G";
    }

    if (event.hp_change != 0) {
        if (event.hp_change > 0) {
            desc += L" +" + std::to_wstring(event.hp_change) + L"HP";
        } else {
            desc += L" " + std::to_wstring(event.hp_change) + L"HP";
        }
    }

    return desc;
}
