#pragma once

#include "../core/GameState.h"
#include "../entity/Player.h"
#include "../entity/Monster.h"
#include "../utils/Types.h"
#include <SFML/Graphics.hpp>
#include <vector>

class UITestState : public GameState {
public:
    void init() override;
    void cleanup() override {}
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void enter_selected();

    sf::Font font_;
    int selected_ = 0;
    std::vector<sf::FloatRect> item_rects_;

    // 测试用数据，保证引用在子状态期间有效
    Player test_player_{0, "UITestPlayer"};
    Monster player_mon_;
    Monster enemy_mon_;
};
