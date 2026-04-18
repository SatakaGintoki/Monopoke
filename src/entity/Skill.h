#pragma once

#include "../utils/Types.h"
#include <string>

struct Skill {
    std::string name;          // 技能名称
    MonsterType type;          // 属性
    int power;                // 威力
    int pp;                   // 当前PP
    int max_pp;               // 最大PP
    float accuracy;           // 命中率 (0.0~1.0)
    SkillCategory category;    // 物理/特殊

    // 使用一次PP
    bool use() {
        if (pp <= 0) return false;
        --pp;
        return true;
    }

    // PP是否用完
    bool exhausted() const { return pp <= 0; }
};
