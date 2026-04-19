#include "Game.h"
#include "states/MenuState.h"
#include "../utils/ConfigManager.h"

Game& Game::get() {
    static Game instance;
    return instance;
}

void Game::init() {
    // 扩大窗口分辨率 (1.5倍)，但保持内部逻辑分辨率 960x640
    window_.create(sf::VideoMode(1440, 960), L"地产兽域 Monopoke", sf::Style::Close);
    window_.setFramerateLimit(60);

    // 设置基础逻辑视图，实现等比例缩放
    base_view_ = sf::View(sf::FloatRect(0, 0, 960, 640));
    window_.setView(base_view_);

    // 加载配置数据
    ConfigManager::get().load_all();

    // 默认启动主菜单
    state_manager_.push_state(new MenuState());
    running_ = true;
}

void Game::run() {
    sf::Clock clock;

    while (window_.isOpen() && running_) {
        delta_time_ = clock.restart().asSeconds();

        handle_events();
        update(delta_time_);
        render();
    }
}

void Game::handle_events() {
    // 每帧事件前重置到基础逻辑视图，避免状态切换后沿用旧 View
    window_.setView(base_view_);

    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }
        if (event.type == sf::Event::KeyPressed &&
            event.key.code == sf::Keyboard::Escape) {
            window_.close();
        }

        // 交给当前状态处理
        state_manager_.handle_event(event);
    }
}

void Game::update(float dt) {
    state_manager_.update(dt);
}

void Game::render() {
    // 每帧渲染前重置基础逻辑视图，防止状态间 View 污染
    window_.setView(base_view_);
    window_.clear(sf::Color(20, 20, 40));
    state_manager_.render(window_);
    window_.display();
}
