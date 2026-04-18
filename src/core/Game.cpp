#include "Game.h"
#include "states/MenuState.h"
#include "../utils/ConfigManager.h"

Game& Game::get() {
    static Game instance;
    return instance;
}

void Game::init() {
    window_.create(sf::VideoMode(960, 640), L"地产兽域 Monopoke", sf::Style::Close);
    window_.setFramerateLimit(60);

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
    window_.clear(sf::Color(20, 20, 40));
    state_manager_.render(window_);
    window_.display();
}
