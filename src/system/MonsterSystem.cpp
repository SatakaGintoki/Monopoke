#include "MonsterSystem.h"
#include "../utils/ConfigManager.h"
#include "../utils/Random.h"
#include <iostream>

using json = nlohmann::json;

// 属性克制表: effectiveness[攻击属性索引][防御属性索引]
// 索引: 0=Normal, 1=Fire, 2=Water, 3=Grass, 4=Electric, 5=Ice, 6=Dragon, 7=Dark, 8=Ghost, 9=Psychic
float MonsterSystem::s_effectiveness_table[10][10] = {
    //      Nor  Fir  Wat  Gra  Ele  Ice  Dra  Dar  Gho  Psy
    /*Nor*/{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0 },
    /*Fire*/{ 1.0, 0.5, 0.5, 2.0, 1.0, 2.0, 0.5, 1.0, 1.0, 1.0 },
    /*Water*/{1.0, 2.0, 0.5, 0.5, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0 },
    /*Grass*/{1.0, 0.5, 2.0, 0.5, 1.0, 1.0, 0.5, 1.0, 1.0, 1.0 },
    /*Elec*/{ 1.0, 1.0, 2.0, 0.5, 0.5, 1.0, 0.5, 1.0, 1.0, 1.0 },
    /*Ice*/{  1.0, 0.5, 0.5, 2.0, 1.0, 0.5, 2.0, 1.0, 1.0, 1.0 },
    /*Drag*/{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 1.0, 1.0, 1.0 },
    /*Dark*/{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 2.0, 2.0 },
    /*Gho*/{  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.5, 2.0, 1.0 },
    /*Psy*/{  1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 2.0, 0.5 }
};

MonsterSystem& MonsterSystem::get() {
    static MonsterSystem instance;
    return instance;
}

void MonsterSystem::init() {
    ConfigManager::get().load_all();
}

MonsterType MonsterSystem::parse_type(const std::string& s) const {
    if (s == "Fire")    return MonsterType::Fire;
    if (s == "Water")   return MonsterType::Water;
    if (s == "Grass")   return MonsterType::Grass;
    if (s == "Electric") return MonsterType::Electric;
    if (s == "Ice")     return MonsterType::Ice;
    if (s == "Dragon")  return MonsterType::Dragon;
    if (s == "Dark")    return MonsterType::Dark;
    if (s == "Ghost")   return MonsterType::Ghost;
    if (s == "Psychic") return MonsterType::Psychic;
    return MonsterType::Normal;
}

Rarity MonsterSystem::parse_rarity(const std::string& s) const {
    if (s == "Rare")       return Rarity::Rare;
    if (s == "Legendary")   return Rarity::Legendary;
    return Rarity::Common;
}

SkillCategory MonsterSystem::parse_category(const std::string& s) const {
    if (s == "Special") return SkillCategory::Special;
    return SkillCategory::Physical;
}

Skill MonsterSystem::create_skill(const json& data) const {
    Skill s;
    s.name = data.value("name", "未知技能");
    s.type = parse_type(data.value("type", "Normal"));
    s.power = data.value("power", 0);
    s.pp = data.value("pp", 10);
    s.max_pp = s.pp;
    s.accuracy = data.value("accuracy", 1.0f);
    s.category = parse_category(data.value("category", "Physical"));
    return s;
}

Monster MonsterSystem::create_from_json(const json& data, int level) const {
    Monster m;
    m.init_from_json(data, level);

    // 技能单独添加
    auto skill_ids = data.value("skills", std::vector<int>{});
    for (int sid : skill_ids) {
        if (auto* tpl = ConfigManager::get().skill_template(sid)) {
            m.add_skill(create_skill(*tpl));
        }
    }

    return m;
}

Monster MonsterSystem::create_monster(int template_id, int level) const {
    if (auto* tpl = ConfigManager::get().monster_template(template_id)) {
        return create_from_json(*tpl, level);
    }
    std::cerr << "[MonsterSystem] Unknown template id: " << template_id << std::endl;
    return Monster();
}

float MonsterSystem::calculate_capture_rate(float hp_ratio, float ball_bonus) const {
    // 基础捕捉率 = 50%（满HP时）
    // 每损失1%HP增加0.5%
    // 球加成：普通球+0%，超级球+20%
    float rate = kCaptureBaseRate + (1.0f - hp_ratio) * kCaptureHpBonus * 100.0f + ball_bonus;
    return std::min(0.95f, rate);
}

bool MonsterSystem::try_capture(const Monster& target, float ball_bonus) const {
    float hp_ratio = static_cast<float>(target.current_hp()) / static_cast<float>(target.max_hp());
    float rate = calculate_capture_rate(hp_ratio, ball_bonus);
    return Random::get().chance() < rate;
}

Monster MonsterSystem::generate_wild_monster() const {
    auto& rng = Random::get();
    int template_id = rng.range(1, 3);   // 小火龙/杰尼龟/妙蛙种子
    int level = rng.range(1, 10);
    return create_monster(template_id, level);
}

std::vector<Monster> MonsterSystem::get_starter_choices() const {
    std::vector<Monster> starters;
    // 初始三选一：1=小火龙, 2=杰尼龟, 3=妙蛙种子，固定等级5
    for (int id = 1; id <= 3; ++id) {
        starters.push_back(create_monster(id, 5));
    }
    return starters;
}

Monster MonsterSystem::get_evolved_form(const Monster& m) const {
    if (m.evolve_to() == 0) return Monster();
    return create_monster(m.evolve_to(), m.level());
}

bool MonsterSystem::try_evolve(Monster& m) const {
    if (!m.can_evolve()) return false;
    Monster evolved = get_evolved_form(m);
    if (evolved.id() == 0) return false;

    m.apply_evolution(evolved);
    return true;
}

float MonsterSystem::get_type_effectiveness(MonsterType atk, MonsterType def) const {
    int ai = static_cast<int>(atk);
    int di = static_cast<int>(def);
    if (ai < 0 || ai >= 10 || di < 0 || di >= 10) return 1.0f;
    return s_effectiveness_table[ai][di];
}
