#pragma once

#include <random>

class Random {
public:
    static Random& get();

    // 掷骰子：1~6
    int roll_dice();

    // 范围随机整数 [min, max]
    int range(int min, int max);

    // 0.0~1.0 随机浮点数
    float chance();

    // 范围随机浮点数 [min, max]
    float range_float(float min, float max);

    // 按概率返回 true/false（传入 0.3 = 30% 概率返回 true）
    bool probability(float rate);

    // 随机挑选一个元素索引
    template<typename T>
    int pick_index(const T& container);

private:
    Random();
    std::mt19937 gen_;
    std::uniform_real_distribution<float> dist_;
};
