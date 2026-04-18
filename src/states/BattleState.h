#pragma once

#include "../core/GameState.h"
#include "../entity/Monster.h"
#include "../system/BattleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>

class BattleState : public GameState {
public:
    BattleState(Monster& player_monster, Monster& enemy_monster, BattleType type, bool ai_controlled = false);

    void init() override;
    void cleanup() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void set_captured_monster(Monster m) { captured_monster_ = m; }
    Monster get_captured_monster() const { return captured_monster_; }

private:
    enum class Phase {
        SelectAction,
        SelectSkill,
        Executing,
        Result
    };

    Phase phase_ = Phase::SelectAction;
    int selected_skill_ = -1;

    Monster& player_monster_;
    Monster& enemy_monster_;
    Monster captured_monster_;
    BattleType battle_type_;
    BattleSystem& battle_sys_;

    sf::Font font_;
    std::wstring battle_message_;
    float result_timer_ = 0.f;
    int player_hp_before_ = 0;
    int enemy_hp_before_ = 0;

    bool ai_controlled_ = false;
    float ai_timer_ = 0.f;
    static constexpr float kAiActionDelay = 0.6f;

    std::wstring get_message_text(int id) const;

    struct Button {
        sf::RectangleShape shape;
        sf::Text text;
        bool hover = false;
    };
    std::vector<Button> action_buttons_;
    std::vector<Button> skill_buttons_;
    Button back_button_;

    void draw_hp_bar(sf::RenderWindow& window, float x, float y, float w, float h,
                     int current, int max, bool is_left);
    void draw_monster_panel(sf::RenderWindow& window, const Monster& mon,
                           float x, float y, bool is_left, bool is_player_mon);
    void create_action_buttons();
    void create_skill_buttons();
    void draw_panel(sf::RenderWindow& window, float x, float y, float w, float h,
                    sf::Color fill = sf::Color(25, 25, 50, 200));
    void draw_label(sf::RenderWindow& window, const sf::String& text, float x, float y,
                   int size, sf::Color color);
    bool contains(const Button& btn, const sf::Vector2f& point) const;
};
