#pragma once

#include "../core/GameState.h"
#include "../entity/Player.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class ShopState : public GameState {
public:
    ShopState(Player& player);

    void init() override;
    void cleanup() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    enum class Tab { Consumable, Evolution };

    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        bool hover = false;
    };

    void refresh_items();
    void draw_item_button(sf::RenderWindow& window, const sf::Vector2f& pos, int index);
    bool contains(const Button& btn, const sf::Vector2f& point) const;

    Player& player_;
    Tab current_tab_ = Tab::Consumable;

    sf::Font font_;
    std::vector<Item> shop_items_;
    std::vector<Item> player_inventory_;

    std::vector<Button> item_buttons_;
    Button tab_buttons_[2];
    Button back_button_;
    Button buy_button_;

    int selected_index_ = -1;
    std::wstring message_;
};
