#pragma once

#include "../core/GameState.h"
#include <vector>

// 主菜单状态（复古像素风格）
class MenuState : public GameState {
public:
    void init() override;
    void cleanup() override {}
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    // 当前高亮选项（0-4）
    int selected_ = 0;

    // 鼠标悬停用的点击区域（与菜单项一一对应）
    std::vector<sf::FloatRect> item_rects_;

    // 动画计时（标题轻微浮动）
    float time_  = 0.f;
};
