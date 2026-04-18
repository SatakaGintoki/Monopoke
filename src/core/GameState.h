#pragma once

#include <SFML/Graphics.hpp>

// 前向声明
class StateManager;

// 所有游戏状态的基类（抽象接口）
// 每个具体状态（MenuState、PlayState等）都要继承它
class GameState {
public:
    virtual ~GameState() = default;

    // 进入状态时调用（初始化）
    virtual void init() = 0;

    // 离开状态时调用（清理）
    virtual void cleanup() = 0;

    // 每帧处理输入事件
    virtual void handle_event(const sf::Event& event) = 0;

    // 每帧逻辑更新（dt = 距离上一帧的秒数）
    virtual void update(float dt) = 0;

    // 每帧渲染到窗口
    virtual void render(sf::RenderWindow& window) = 0;

    // 被子状态压栈时暂停
    virtual void pause() {}

    // 子状态弹出后恢复
    virtual void resume() {}

    // 暂停当前状态，切换到新状态（压栈）
    void push_state(StateManager& mgr, GameState* new_state);

    // 弹出当前状态，返回上一个状态
    void pop_state(StateManager& mgr);

    // 替换当前状态（不保留旧状态）
    void switch_state(StateManager& mgr, GameState* new_state);
};
