#pragma once

#include "../utils/Types.h"
#include "Skill.h"
#include "json.hpp"
#include <string>
#include <vector>

class Monster {
public:
    Monster() = default;

    // 从模板ID创建精灵
    static Monster create_from_template(int template_id, int level);

    // 基础属性
    int id() const { return id_; }
    const std::string& name() const { return name_; }
    MonsterType type() const { return type_; }
    int level() const { return level_; }

    // HP
    int current_hp() const { return current_hp_; }
    int max_hp() const { return max_hp_; }
    void heal(int amount);
    void take_damage(int dmg);
    bool is_alive() const { return current_hp_ > 0; }

    // 战斗属性
    int atk() const { return atk_; }
    int def() const { return def_; }
    int spd() const { return spd_; }
    int sp_atk() const { return sp_atk_; }
    int sp_def() const { return sp_def_; }

    // 经验
    int exp() const { return exp_; }
    int exp_to_next() const { return exp_to_next_; }
    bool gain_exp(int amount);
    void level_up();

    // 进化
    int evolve_to() const { return evolve_to_id_; }
    int evolve_level() const { return evolve_level_; }
    bool can_evolve() const;
    void evolve_to_level(int lvl) { evolve_level_ = lvl; }
    void clear_evolution() { evolve_to_id_ = 0; evolve_level_ = 0; }
    void apply_evolution(const Monster& evolved);

    // 从JSON初始化（供MonsterSystem使用）
    void init_from_json(const nlohmann::json& data, int level);

    // 私有字段Setter（供MonsterSystem使用）
    void set_id(int v) { id_ = v; }
    void set_name(const std::string& v) { name_ = v; }
    void set_type(MonsterType v) { type_ = v; }
    void set_level(int v) { level_ = v; }
    void set_current_hp(int v) { current_hp_ = v; }
    void set_max_hp(int v) { max_hp_ = v; }
    void set_atk(int v) { atk_ = v; }
    void set_def(int v) { def_ = v; }
    void set_spd(int v) { spd_ = v; }
    void set_sp_atk(int v) { sp_atk_ = v; }
    void set_sp_def(int v) { sp_def_ = v; }
    void set_exp(int v) { exp_ = v; }
    void set_exp_to_next(int v) { exp_to_next_ = v; }
    void set_rarity(Rarity v) { rarity_ = v; }
    void add_skill(const Skill& s) { skills_.push_back(s); }

    // 技能
    const std::vector<Skill>& skills() const { return skills_; }
    void clear_skills() { skills_.clear(); }
    bool use_skill(int index);

    // 稀有度
    Rarity rarity() const { return rarity_; }

    // JSON序列化
    std::string to_json() const;
    void from_json(const std::string& json);

private:
    int id_ = 0;
    std::string name_;
    MonsterType type_ = MonsterType::Normal;
    int level_ = 1;
    int current_hp_ = 100;
    int max_hp_ = 100;
    int atk_ = 50;
    int def_ = 50;
    int spd_ = 50;
    int sp_atk_ = 50;
    int sp_def_ = 50;
    int exp_ = 0;
    int exp_to_next_ = 10;
    int evolve_to_id_ = 0;
    int evolve_level_ = 0;
    Rarity rarity_ = Rarity::Common;
    std::vector<Skill> skills_;
};
