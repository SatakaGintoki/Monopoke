#include "ResultState.h"
#include "../core/Game.h"
#include "MenuState.h"

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
    window.clear(sf::Color(15, 15, 35));

    // Background bands
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape band({960.f, 80.f});
        band.setPosition(0.f, i * 80.f);
        sf::Uint8 r = static_cast<sf::Uint8>(15 + i * 2);
        band.setFillColor(sf::Color(r, r, 35 + i * 4));
        window.draw(band);
    }

    // Result panel
    sf::RectangleShape panel({600.f, 300.f});
    panel.setPosition(180.f, 170.f);
    panel.setFillColor(sf::Color(25, 25, 50, 230));
    panel.setOutlineThickness(2.f);
    panel.setOutlineColor(sf::Color(60, 60, 120, 150));
    window.draw(panel);

    // Title
    sf::Text title;
    title.setFont(font_);
    if (result_ == GameResult::Player1Win) {
        title.setString(L"\u73a9\u5bb61 \u83b7\u80dc\uff01");
        title.setFillColor(sf::Color(80, 180, 255));
    } else if (result_ == GameResult::Player2Win) {
        title.setString(L"\u73a9\u5bb62 \u83b7\u80dc\uff01");
        title.setFillColor(sf::Color(255, 140, 80));
    } else {
        title.setString(L"\u5e73\u5c40\uff01");
        title.setFillColor(sf::Color(200, 200, 100));
    }
    title.setCharacterSize(36);
    auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(480.f, 220.f);
    window.draw(title);

    // Subtitle
    sf::Text subtitle;
    subtitle.setFont(font_);
    subtitle.setString(L"\u611f\u8c22\u53c2\u4e0e\uff01");
    subtitle.setCharacterSize(20);
    subtitle.setFillColor(sf::Color(180, 180, 200));
    auto sb = subtitle.getLocalBounds();
    subtitle.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    subtitle.setPosition(480.f, 290.f);
    window.draw(subtitle);

    // Continue hint
    sf::Text hint;
    hint.setFont(font_);
    hint.setString(L"\u70b9\u51fb\u5c4f\u5e55\u6216\u6309 Enter \u8fd4\u56de\u83dc\u5355");
    hint.setCharacterSize(16);
    hint.setFillColor(sf::Color(100, 100, 140));
    auto hb = hint.getLocalBounds();
    hint.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
    hint.setPosition(480.f, 400.f);
    window.draw(hint);
}
