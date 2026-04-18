#pragma once

#include "../core/GameState.h"
#include <vector>

// 按钮结构
struct Button {
    sf::RectangleShape shape;
    sf::RectangleShape glow;       // 悬停发光层
    sf::Text text;
    bool hover = false;
    float hover_anim = 0.f;        // 悬停动画进度 0~1

    bool contains(const sf::Vector2f& point) const;
};

// 背景粒子（漂浮装饰）
struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float alpha;
    float alpha_speed;
};

// 主菜单状态
class MenuState : public GameState {
public:
    void init() override;
    void cleanup() override {}
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void create_buttons();
    void create_particles();
    void draw_background(sf::RenderWindow& window);

    sf::Font font_;
    std::vector<Button> buttons_;
    sf::Text title_;
    sf::Text subtitle_;
    sf::Text version_;

    // 背景装饰
    std::vector<Particle> particles_;
    std::vector<sf::RectangleShape> bg_lines_;

    // 动画
    float time_ = 0.f;
    float title_bounce_ = 0.f;
};
