#include "Monster.h"
#include "Skill.h"
#include <map>

using json = nlohmann::json;

void Monster::heal(int amount) {
    current_hp_ = std::min(max_hp_, current_hp_ + amount);
}

bool Monster::use_skill(int index) {
    if (index < 0 || index >= static_cast<int>(skills_.size())) return false;
    return skills_[index].use();
}

void Monster::take_damage(int dmg) {
    current_hp_ = std::max(0, current_hp_ - dmg);
}

bool Monster::gain_exp(int amount) {
    exp_ += amount;
    bool leveled = false;
    while (exp_ >= exp_to_next_ && level_ < kMaxLevel) {
        exp_ -= exp_to_next_;
        level_up();
        leveled = true;
    }
    return leveled;
}

void Monster::level_up() {
    if (level_ >= kMaxLevel) return;
    ++level_;

    // 每级属性+5%
    float ratio = 1.05f;
    atk_ = static_cast<int>(atk_ * ratio);
    def_ = static_cast<int>(def_ * ratio);
    spd_ = static_cast<int>(spd_ * ratio);
    sp_atk_ = static_cast<int>(sp_atk_ * ratio);
    sp_def_ = static_cast<int>(sp_def_ * ratio);
    max_hp_ = static_cast<int>(max_hp_ * ratio);
    current_hp_ = std::min(current_hp_, max_hp_);

    exp_to_next_ = level_ * 10;
}

bool Monster::can_evolve() const {
    return evolve_to_id_ > 0 && level_ >= evolve_level_;
}

void Monster::apply_evolution(const Monster& evolved) {
    name_ = evolved.name_;
    type_ = evolved.type_;
    evolve_to_id_ = 0;
    evolve_level_ = 0;
}

void Monster::init_from_json(const json& data, int level) {
    static const std::map<std::string, MonsterType> type_map = {
        {"Fire", MonsterType::Fire}, {"Water", MonsterType::Water},
        {"Grass", MonsterType::Grass}, {"Electric", MonsterType::Electric},
        {"Ice", MonsterType::Ice}, {"Dragon", MonsterType::Dragon},
        {"Dark", MonsterType::Dark}, {"Ghost", MonsterType::Ghost},
        {"Psychic", MonsterType::Psychic}
    };
    static const std::map<std::string, Rarity> rarity_map = {
        {"Rare", Rarity::Rare}, {"Legendary", Rarity::Legendary}
    };

    id_ = data.value("id", 0);
    name_ = data.value("name", "未知精灵");
    auto it = type_map.find(data.value("type", "Normal"));
    type_ = (it != type_map.end()) ? it->second : MonsterType::Normal;
    level_ = level;
    rarity_ = Rarity::Common;

    int base_hp = data.value("base_hp", 100);
    int base_atk = data.value("base_atk", 50);
    int base_def = data.value("base_def", 50);
    int base_spd = data.value("base_spd", 50);
    int base_sp_atk = data.value("base_sp_atk", 50);
    int base_sp_def = data.value("base_sp_def", 50);

    float scale = 0.5f + static_cast<float>(level) * 0.05f;
    max_hp_ = static_cast<int>(base_hp * scale);
    current_hp_ = max_hp_;
    atk_ = static_cast<int>(base_atk * scale);
    def_ = static_cast<int>(base_def * scale);
    spd_ = static_cast<int>(base_spd * scale);
    sp_atk_ = static_cast<int>(base_sp_atk * scale);
    sp_def_ = static_cast<int>(base_sp_def * scale);

    evolve_to_id_ = data.value("evolve_to_id", 0);
    evolve_level_ = data.value("evolve_level", 0);
    exp_to_next_ = level * 10;
    exp_ = 0;

    skills_.clear();
}

Monster Monster::create_from_template(int template_id, int level) {
    // 实际创建使用 MonsterSystem::create_monster()
    // 这里返回默认精灵，由调用方负责正确初始化
    Monster m;
    m.id_ = template_id;
    m.level_ = level;
    m.max_hp_ = 50 + level * 5;
    m.current_hp_ = m.max_hp_;
    m.atk_ = 20 + level * 3;
    m.def_ = 20 + level * 3;
    m.spd_ = 20 + level * 3;
    m.exp_to_next_ = level * 10;
    return m;
}

std::string Monster::to_json() const {
    return "{}";  // TODO: 完整序列化
}

void Monster::from_json(const std::string& json) {
    (void)json;  // TODO: 反序列化
}
