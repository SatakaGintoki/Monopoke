#pragma once

#include <stack>
#include <memory>
#include "GameState.h"

// 栈式状态管理器
// - push_state:   暂停当前，压入新的（游戏里打开商店/战斗）
// - pop_state:    弹出当前，恢复上一个（关闭商店/战斗结束）
// - switch_state: 直接替换（主菜单→游戏中）
class StateManager {
public:
    StateManager();

    // 检查是否还有状态
    bool empty() const;

    // 获取当前状态
    GameState* current() const;

    // 压入新状态（暂停当前）
    void push_state(GameState* state);

    // 弹出当前状态
    void pop_state();

    // 直接替换当前状态
    void switch_state(GameState* state);

    // 每帧调用：处理事件
    void handle_event(const sf::Event& event);

    // 每帧调用：逻辑更新
    void update(float dt);

    // 每帧调用：渲染
    void render(sf::RenderWindow& window);

private:
    std::stack<std::unique_ptr<GameState>> states_;
};
