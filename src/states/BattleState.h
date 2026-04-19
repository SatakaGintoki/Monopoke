#pragma once

#include "../core/GameState.h"
#include "../entity/Monster.h"
#include "../system/BattleSystem.h"
#include <SFML/Graphics.hpp>
#include <vector>

struct BattleExitSnapshot {
    bool valid = false;
    BattleResult result = BattleResult::Ongoing;
    BattleType type = BattleType::Wild;
    int gold_reward = 0;
};

class BattleState : public GameState {
public:
    BattleState(Monster& player_monster, Monster& enemy_monster, BattleType type, bool ai_controlled = false);

    void init() override;
    void cleanup() override;

    // 供 PlayState::resume 读取上一场战斗结果（pop 时 cleanup 写入）
    static BattleExitSnapshot consume_exit_snapshot();
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

    static BattleExitSnapshot s_last_exit_;

    void draw_status_box(sf::RenderWindow& window, const Monster& mon,
                         float x, float y, bool is_player_mon);
    void create_action_buttons();
    void create_skill_buttons();
    bool contains(const Button& btn, const sf::Vector2f& point) const;

    // 贴图支持
    sf::Texture bg_texture_;
    sf::Texture player_mon_tex_;
    sf::Texture enemy_mon_tex_;
    sf::View ui_view_;
};
