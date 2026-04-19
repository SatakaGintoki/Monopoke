#include "ResultState.h"
#include "../core/Game.h"
#include "MenuState.h"
#include "../ui/RetroUI.h"

ResultState::ResultState(GameResult result, int winner_id)
    : result_(result), winner_id_(winner_id) {}

void ResultState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }
}

void ResultState::cleanup() {}

void ResultState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseButtonPressed ||
        (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter)) {
        // 返回主菜单
        Game::get().state_manager().switch_state(new MenuState());
    }
}

void ResultState::update(float dt) {}

void ResultState::render(sf::RenderWindow& window) {
    // 纯色背景
    sf::RectangleShape bg({1440.f, 960.f});
    bg.setFillColor(sf::Color(248, 248, 240));
    window.draw(bg);

    // Result panel
    RetroUI::draw_box(window, 180.f, 170.f, 600.f, 300.f, sf::Color::White, sf::Color(40, 40, 40), 4.f);

    // Title
    std::wstring title_str;
    if (result_ == GameResult::Player1Win) {
        title_str = L"玩家1 获胜！";
    } else if (result_ == GameResult::Player2Win) {
        title_str = L"玩家2 获胜！";
    } else {
        title_str = L"平局！";
    }
    RetroUI::draw_text(window, title_str, 480.f - 80.f, 220.f, 36, sf::Color(40, 40, 40), true);

    // Subtitle
    RetroUI::draw_text(window, L"感谢参与！", 480.f - 50.f, 290.f, 20, sf::Color(100, 100, 100));

    // Continue hint
    RetroUI::draw_text(window, L"点击屏幕或按 Enter 返回菜单", 480.f - 120.f, 400.f, 16, sf::Color(80, 80, 80));
}
