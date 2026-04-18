#pragma once

#include "../entity/Player.h"
#include "../utils/Types.h"
#include <string>
#include <vector>

struct RandomEvent {
    EventType type;
    std::string name;
    std::string description;
    int gold_change = 0;
    int hp_change = 0;
    bool capture_bonus = false;
    bool monster_lost = false;
    float probability = 0.0f;
};

class EventSystem {
public:
    static EventSystem& get();

    // 触发随机事件
    RandomEvent trigger_event(Player& player);

    // 获取事件结果描述
    std::wstring get_event_description(const RandomEvent& event) const;

    // 加载事件数据
    void load_from_json(const std::string& json_path);

private:
    EventSystem() = default;

    std::vector<RandomEvent> events_;
};
