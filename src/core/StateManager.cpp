#include "StateManager.h"
#include <cassert>

StateManager::StateManager() = default;

bool StateManager::empty() const {
    return states_.empty();
}

GameState* StateManager::current() const {
    return states_.empty() ? nullptr : states_.top().get();
}

void StateManager::push_state(GameState* state) {
    if (!states_.empty()) {
        states_.top()->pause();  // 暂停当前状态（不销毁）
    }
    states_.push(std::unique_ptr<GameState>(state));
    states_.top()->init();  // 初始化新状态
}

void StateManager::pop_state() {
    if (states_.empty()) return;

    states_.top()->cleanup();  // 清理当前状态
    states_.pop();             // unique_ptr 自动释放内存

    if (!states_.empty()) {
        states_.top()->resume(); // 恢复上一个状态
    }
}

void StateManager::switch_state(GameState* state) {
    // 先清空栈，再压入新状态
    while (!states_.empty()) {
        states_.top()->cleanup();
        states_.pop();
    }
    states_.push(std::unique_ptr<GameState>(state));
    states_.top()->init();
}

void StateManager::handle_event(const sf::Event& event) {
    if (!states_.empty()) {
        states_.top()->handle_event(event);
    }
}

void StateManager::update(float dt) {
    if (!states_.empty()) {
        states_.top()->update(dt);
    }
}

void StateManager::render(sf::RenderWindow& window) {
    if (!states_.empty()) {
        states_.top()->render(window);
    }
}
