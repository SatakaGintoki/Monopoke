#include "MenuState.h"
#include "PlayState.h"
#include "../core/Game.h"
#include "../utils/Random.h"
#include "../utils/SaveManager.h"
#include <cmath>

bool Button::contains(const sf::Vector2f& point) const {
    return shape.getGlobalBounds().contains(point);
}

void MenuState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    // 标题
    title_.setFont(font_);
    title_.setString(L"\u5730\u4ea7\u517d\u57df");
    title_.setCharacterSize(64);
    title_.setFillColor(sf::Color(255, 220, 100));
    title_.setStyle(sf::Text::Bold);
    auto tb = title_.getLocalBounds();
    title_.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title_.setPosition(480.f, 160.f);

    // 副标题
    subtitle_.setFont(font_);
    subtitle_.setString("M O N O P O K E");
    subtitle_.setCharacterSize(20);
    subtitle_.setFillColor(sf::Color(180, 180, 220));
    subtitle_.setLetterSpacing(3.f);
    auto sb = subtitle_.getLocalBounds();
    subtitle_.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    subtitle_.setPosition(480.f, 215.f);

    // 版本号
    version_.setFont(font_);
    version_.setString("v0.2.0");
    version_.setCharacterSize(12);
    version_.setFillColor(sf::Color(100, 100, 130));
    version_.setPosition(900.f, 620.f);

    create_buttons();
    create_particles();

    // 背景装饰线
    for (int i = 0; i < 6; ++i) {
        sf::RectangleShape line({960.f, 1.f});
        line.setFillColor(sf::Color(40, 40, 70, 40));
        line.setPosition(0.f, 80.f + i * 100.f);
        bg_lines_.push_back(line);
    }
}

void MenuState::create_buttons() {
    buttons_.clear();

    const wchar_t* labels[] = {
        L"\u5f00\u59cb\u6e38\u620f",
        L"\u7ee7\u7eed\u6e38\u620f",
        L"\u9000\u51fa"
    };

    for (int i = 0; i < 3; ++i) {
        Button btn;

        // 主体
        btn.shape.setSize({260.f, 52.f});
        btn.shape.setPosition(350.f, 300.f + i * 72.f);
        btn.shape.setFillColor(sf::Color(35, 35, 65));
        btn.shape.setOutlineThickness(1.5f);
        btn.shape.setOutlineColor(sf::Color(80, 80, 140, 120));

        // 发光层
        btn.glow.setSize({264.f, 56.f});
        btn.glow.setPosition(348.f, 298.f + i * 72.f);
        btn.glow.setFillColor(sf::Color(100, 120, 255, 0));

        // 文字
        btn.text.setFont(font_);
        btn.text.setString(labels[i]);
        btn.text.setCharacterSize(22);
        btn.text.setFillColor(sf::Color(200, 200, 230));
        auto lb = btn.text.getLocalBounds();
        btn.text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        btn.text.setPosition(480.f, 326.f + i * 72.f);

        buttons_.push_back(btn);
    }
}

void MenuState::create_particles() {
    particles_.clear();
    auto& rng = Random::get();

    for (int i = 0; i < 40; ++i) {
        Particle p;
        float radius = 1.f + rng.range_float(0.f, 2.5f);
        p.shape.setRadius(radius);
        p.shape.setPosition(rng.range_float(0.f, 960.f), rng.range_float(0.f, 640.f));
        p.velocity = {rng.range_float(-8.f, 8.f), rng.range_float(-12.f, -3.f)};
        p.alpha = rng.range_float(20.f, 80.f);
        p.alpha_speed = rng.range_float(15.f, 40.f);
        p.shape.setFillColor(sf::Color(140, 160, 255, static_cast<sf::Uint8>(p.alpha)));
        particles_.push_back(p);
    }
}

void MenuState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse(static_cast<float>(event.mouseMove.x),
                           static_cast<float>(event.mouseMove.y));
        for (auto& btn : buttons_) {
            btn.hover = btn.contains(mouse);
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse(static_cast<float>(event.mouseButton.x),
                           static_cast<float>(event.mouseButton.y));
        for (int i = 0; i < static_cast<int>(buttons_.size()); ++i) {
            if (buttons_[i].contains(mouse)) {
                if (i == 0) switch_state(Game::get().state_manager(), new PlayState());
                if (i == 1) {
                    // 继续游戏：检查存档是否存在
                    if (SaveManager::get().has_save()) {
                        // TODO: 加载存档并开始游戏
                        switch_state(Game::get().state_manager(), new PlayState());
                    }
                }
                if (i == 2) Game::get().exit();
            }
        }
    }
}

void MenuState::update(float dt) {
    time_ += dt;

    // 标题浮动动画
    title_bounce_ = std::sin(time_ * 1.5f) * 6.f;
    title_.setPosition(480.f, 160.f + title_bounce_);

    // 副标题跟随
    subtitle_.setPosition(480.f, 215.f + title_bounce_ * 0.4f);

    // 按钮悬停动画
    for (auto& btn : buttons_) {
        float target = btn.hover ? 1.f : 0.f;
        float speed = 5.f * dt;
        if (btn.hover_anim < target) btn.hover_anim = std::min(btn.hover_anim + speed, target);
        else btn.hover_anim = std::max(btn.hover_anim - speed, target);

        // 插值颜色
        sf::Uint8 r = static_cast<sf::Uint8>(35 + btn.hover_anim * 30);
        sf::Uint8 g = static_cast<sf::Uint8>(35 + btn.hover_anim * 30);
        sf::Uint8 b = static_cast<sf::Uint8>(65 + btn.hover_anim * 50);
        btn.shape.setFillColor(sf::Color(r, g, b));

        sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + btn.hover_anim * 135);
        btn.shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));

        sf::Uint8 glow_a = static_cast<sf::Uint8>(btn.hover_anim * 30);
        btn.glow.setFillColor(sf::Color(80, 100, 220, glow_a));

        sf::Uint8 text_brightness = static_cast<sf::Uint8>(200 + btn.hover_anim * 55);
        btn.text.setFillColor(sf::Color(text_brightness, text_brightness, 255));
    }

    // 粒子动画
    for (auto& p : particles_) {
        p.shape.move(p.velocity * dt);
        p.alpha += p.alpha_speed * dt;
        if (p.alpha > 90.f) { p.alpha_speed = -std::abs(p.alpha_speed); }
        if (p.alpha < 10.f) { p.alpha_speed = std::abs(p.alpha_speed); }
        p.shape.setFillColor(sf::Color(140, 160, 255, static_cast<sf::Uint8>(p.alpha)));

        // 超出屏幕则重置
        auto pos = p.shape.getPosition();
        if (pos.y < -10.f) {
            p.shape.setPosition(Random::get().range_float(0.f, 960.f), 650.f);
        }
        if (pos.x < -10.f || pos.x > 970.f) {
            p.shape.setPosition(Random::get().range_float(0.f, 960.f), 650.f);
        }
    }
}

void MenuState::draw_background(sf::RenderWindow& window) {
    // 渐变背景（用多层矩形模拟）
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape band({960.f, 80.f});
        band.setPosition(0.f, i * 80.f);
        sf::Uint8 r = static_cast<sf::Uint8>(15 + i * 2);
        sf::Uint8 g = static_cast<sf::Uint8>(15 + i * 2);
        sf::Uint8 b = static_cast<sf::Uint8>(30 + i * 5);
        band.setFillColor(sf::Color(r, g, b));
        window.draw(band);
    }

    // 装饰线
    for (auto& line : bg_lines_) {
        window.draw(line);
    }

    // 粒子
    for (auto& p : particles_) {
        window.draw(p.shape);
    }

    // 中央装饰框
    sf::RectangleShape frame({320.f, 380.f});
    frame.setPosition(320.f, 120.f);
    frame.setFillColor(sf::Color(20, 20, 45, 120));
    frame.setOutlineThickness(1.f);
    frame.setOutlineColor(sf::Color(60, 60, 120, 80));
    window.draw(frame);
}

void MenuState::render(sf::RenderWindow& window) {
    draw_background(window);

    // 标题
    window.draw(title_);
    window.draw(subtitle_);

    // 按钮
    for (auto& btn : buttons_) {
        window.draw(btn.glow);
        window.draw(btn.shape);
        window.draw(btn.text);
    }

    // 底部装饰线
    sf::RectangleShape bottom_line({400.f, 1.f});
    bottom_line.setPosition(280.f, 530.f);
    bottom_line.setFillColor(sf::Color(60, 60, 120, 60));
    window.draw(bottom_line);

    // 版本号
    window.draw(version_);
}
