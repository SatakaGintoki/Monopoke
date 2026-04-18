#pragma once

#include "../entity/Monster.h"
#include "../entity/Skill.h"
#include "../utils/Types.h"
#include <vector>

struct BattleInfo {
    Monster* attacker = nullptr;
    Monster* defender = nullptr;
    BattleType type = BattleType::Wild;
    bool attacker_turn = true;  // true=攻击方先手
    int turn_count = 0;
};

struct TurnAction {
    enum Type { UseSkill, Capture, Escape } type;
    int skill_index = -1;      // 技能索引（UseSkill时）
    int ball_bonus = 0;         // 球的加成（Capture时）
};

struct BattleResultInfo {
    BattleResult result = BattleResult::Ongoing;
    int exp_gained = 0;
    int gold_reward = 0;
    bool captured = false;
    int message_id = 0;  // 0=空, 1=逃跑成功, 2=逃跑失败, etc.
};

class BattleSystem {
public:
    static BattleSystem& get();

    // 开始战斗
    void start_battle(Monster& attacker, Monster& defender, BattleType type);

    // 执行一回合行动
    BattleResult execute_action(const TurnAction& action);

    // 计算伤害
    int calculate_damage(Monster& attacker, Monster& defender, const Skill& skill) const;

    // 属性克制
    float get_type_effectiveness(MonsterType atk, MonsterType def) const;

    // 尝试捕捉
    bool try_capture(Monster& target, int ball_bonus) const;

    // 尝试逃跑
    bool try_escape(Monster& attacker, Monster& defender) const;

    // 获取当前战斗信息
    BattleInfo info() const { return info_; }
    BattleResultInfo result() const { return result_; }

    // 伤害数字记录（供UI显示）
    int last_damage() const { return last_damage_; }

    // 攻击方是否先手
    bool attacker_first() const;

private:
    BattleSystem() = default;

    BattleInfo info_;
    BattleResultInfo result_;
    mutable int last_damage_ = 0;

    // 逃跑成功率
    static constexpr float kEscapeBaseRate = 0.75f;
};
