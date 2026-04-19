#pragma once

#include <SFML/Graphics.hpp>
#include "StateManager.h"

class Game {
public:
    // 获取唯一实例（单例）
    static Game& get();

    // 禁止拷贝
    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    // 初始化游戏
    void init();

    // 主循环入口
    void run();

    // 获取窗口
    sf::RenderWindow& window() { return window_; }

    // 获取状态管理器
    StateManager& state_manager() { return state_manager_; }

    // 退出游戏
    void exit() { running_ = false; }

private:
    Game() = default;

    void handle_events();   // 处理输入事件
    void update(float dt);  // 更新逻辑
    void render();          // 渲染画面

    sf::RenderWindow window_;
    sf::View base_view_;
    StateManager state_manager_;
    bool running_ = false;
    float delta_time_ = 0.0f;

    static constexpr float kTargetFPS = 60.0f;
};
