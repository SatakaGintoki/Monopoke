#include "Random.h"
#include <algorithm>

Random::Random() : gen_(std::random_device{}()), dist_(0.0f, 1.0f) {}

Random& Random::get() {
    static Random instance;
    return instance;
}

int Random::roll_dice() {
    return range(1, 6);
}

int Random::range(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
}

float Random::chance() {
    return dist_(gen_);
}

bool Random::probability(float rate) {
    return chance() < rate;
}

float Random::range_float(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen_);
}

template<typename T>
int Random::pick_index(const T& container) {
    if (container.empty()) return -1;
    return range(0, static_cast<int>(container.size()) - 1);
}
