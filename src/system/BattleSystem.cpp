#include "BattleSystem.h"
#include "../utils/Random.h"
#include <cmath>

BattleSystem& BattleSystem::get() {
    static BattleSystem instance;
    return instance;
}

void BattleSystem::start_battle(Monster& attacker, Monster& defender, BattleType type) {
    info_.attacker = &attacker;
    info_.defender = &defender;
    info_.type = type;
    info_.turn_count = 0;
    last_damage_ = 0;

    // 速度快的先手
    info_.attacker_turn = (attacker.spd() >= defender.spd());

    result_.result = BattleResult::Ongoing;
    result_.exp_gained = 0;
    result_.gold_reward = 0;
    result_.captured = false;
    result_.message_id = 0;
}

bool BattleSystem::attacker_first() const {
    return info_.attacker_turn;
}

float BattleSystem::get_type_effectiveness(MonsterType atk, MonsterType def) const {
    // 属性克制表
    static const float table[10][10] = {
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
    int ai = static_cast<int>(atk);
    int di = static_cast<int>(def);
    if (ai < 0 || ai >= 10 || di < 0 || di >= 10) return 1.0f;
    return table[ai][di];
}

int BattleSystem::calculate_damage(Monster& attacker, Monster& defender, const Skill& skill) const {
    if (skill.power == 0) return 0;

    int atk_val = (skill.category == SkillCategory::Physical) ? attacker.atk() : attacker.sp_atk();
    int def_val = (skill.category == SkillCategory::Physical) ? defender.def() : defender.sp_def();

    float effectiveness = get_type_effectiveness(skill.type, defender.type());
    float random_factor = Random::get().range_float(0.85f, 1.0f);

    int damage = static_cast<int>(std::floor(
        (atk_val * skill.power / static_cast<float>(def_val))
        * effectiveness * random_factor
    ));

    last_damage_ = std::max(1, damage);
    return last_damage_;
}

bool BattleSystem::try_capture(Monster& target, int ball_bonus) const {
    float hp_ratio = static_cast<float>(target.current_hp()) / static_cast<float>(target.max_hp());
    float ball = (ball_bonus > 0) ? 0.2f : 0.0f;
    float rate = kCaptureBaseRate + (1.0f - hp_ratio) * kCaptureHpBonus * 100.0f + ball;
    rate = std::min(0.95f, rate);
    return Random::get().chance() < rate;
}

bool BattleSystem::try_escape(Monster& attacker, Monster& defender) const {
    (void)attacker;
    (void)defender;
    return Random::get().chance() < kEscapeBaseRate;
}

BattleResult BattleSystem::execute_action(const TurnAction& action) {
    if (result_.result != BattleResult::Ongoing) {
        return result_.result;
    }

    ++info_.turn_count;

    Monster* actor = info_.attacker_turn ? info_.attacker : info_.defender;
    Monster* target = info_.attacker_turn ? info_.defender : info_.attacker;

    if (action.type == TurnAction::Escape) {
        if (info_.type == BattleType::Wild) {
            if (try_escape(*actor, *target)) {
                result_.result = BattleResult::Escaped;
                result_.message_id = 1;  // 逃跑成功
            } else {
                result_.message_id = 2;  // 逃跑失败
                info_.attacker_turn = !info_.attacker_turn;
            }
        } else {
            result_.message_id = 3;  // 无法逃跑
            info_.attacker_turn = !info_.attacker_turn;
        }
        return result_.result;
    }

    if (action.type == TurnAction::Capture) {
        if (info_.type == BattleType::Wild) {
            if (try_capture(*target, action.ball_bonus)) {
                result_.result = BattleResult::Captured;
                result_.captured = true;
                result_.message_id = 4;  // 捕捉成功
            } else {
                result_.message_id = 5;  // 捕捉失败
                info_.attacker_turn = !info_.attacker_turn;
            }
        } else {
            result_.message_id = 6;  // 无法捕捉
            info_.attacker_turn = !info_.attacker_turn;
        }
        return result_.result;
    }

    // 使用技能
    if (action.type == TurnAction::UseSkill && action.skill_index >= 0) {
        auto& skills = actor->skills();
        if (action.skill_index >= static_cast<int>(skills.size())) {
            info_.attacker_turn = !info_.attacker_turn;
            return result_.result;
        }

        const Skill& skill = skills[action.skill_index];

        if (skill.exhausted()) {
            result_.message_id = 7;  // PP不足
            info_.attacker_turn = !info_.attacker_turn;
            return result_.result;
        }

        if (Random::get().chance() > skill.accuracy) {
            result_.message_id = 8;  // miss
            info_.attacker_turn = !info_.attacker_turn;
            return result_.result;
        }

        int damage = calculate_damage(*actor, *target, skill);
        target->take_damage(damage);
        actor->use_skill(action.skill_index);

        info_.attacker_turn = !info_.attacker_turn;

        if (!target->is_alive()) {
            if (info_.type == BattleType::Wild) {
                int exp_reward = target->level() * 10 + 20;
                actor->gain_exp(exp_reward);
                result_.exp_gained = exp_reward;
                result_.gold_reward = target->level() * 5;
                result_.result = BattleResult::Win;
                result_.message_id = 9;  // 胜利
            } else {
                result_.result = (target == info_.defender) ? BattleResult::Win : BattleResult::Lose;
                result_.message_id = (result_.result == BattleResult::Win) ? 9 : 10;  // 胜利/失败
            }
        }
    }

    return result_.result;
}
