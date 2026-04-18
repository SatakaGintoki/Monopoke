#pragma once

#include "../entity/Monster.h"
#include "../entity/Skill.h"
#include "../utils/Types.h"
#include "json.hpp"
#include <vector>

class MonsterSystem {
public:
    static MonsterSystem& get();

    // 初始化（加载JSON数据）
    void init();

    // 从模板ID和等级创建精灵
    Monster create_monster(int template_id, int level) const;

    // 从JSON创建精灵（供load_all使用）
    Monster create_from_json(const nlohmann::json& data, int level) const;

    // 计算捕捉概率
    // hp_ratio: 0.0(空血)~1.0(满血), ball_bonus: 0.0(普通球)~0.2(超级球)
    float calculate_capture_rate(float hp_ratio, float ball_bonus) const;

    // 尝试捕捉，返回是否成功
    bool try_capture(const Monster& target, float ball_bonus) const;

    // 生成野生精灵（随机模板ID 1-3，随机等级 1-10）
    Monster generate_wild_monster() const;

    // 获取初始三选一精灵列表
    std::vector<Monster> get_starter_choices() const;

    // 检查并执行进化
    bool try_evolve(Monster& m) const;

    // 获取进化后的精灵模板
    Monster get_evolved_form(const Monster& m) const;

    // 属性克制表查询
    float get_type_effectiveness(MonsterType atk, MonsterType def) const;

private:
    MonsterSystem() = default;

    MonsterType parse_type(const std::string& s) const;
    Rarity parse_rarity(const std::string& s) const;
    SkillCategory parse_category(const std::string& s) const;
    Skill create_skill(const nlohmann::json& data) const;

    // 属性克制表
    static float s_effectiveness_table[10][10];
};
