#pragma once

#include "../core/GameState.h"
#include "../entity/Board.h"
#include "../entity/Player.h"
#include "../render/BoardRenderer.h"
#include "../utils/Random.h"
#include "../utils/Types.h"

// 回合阶段
enum class TurnPhase {
    WaitRoll,       // 等待掷骰
    Moving,         // 棋子移动动画
    TileAction,     // 格子事件处理
    WildChoice,     // 野怪格：捕捉 / 战斗 / 逃跑
    CampChoice,     // 营地：F1-F6 治疗该精灵满血，Backspace 放生最后一只 +50 金
    ArenaChoice,    // 对战格：1 开战 / 2 逃跑
    TurnEnd         // 回合结束，切换玩家
};

class PlayState : public GameState {
public:
    explicit PlayState(bool load_from_save = false, GameMode mode = GameMode::PvE);

    void init() override;
    void cleanup() override {}
    void resume() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    enum class PendingBattle { None, GuardianTile, ArenaPvP };

    void init_new_game();
    void roll_dice();
    void next_turn();
    void on_tile_landed();
    int check_game_over() const;  // 返回赢家ID，-1表示游戏继续

    void resolve_guardian_after_battle(BattleResult r);
    void resolve_arena_after_battle(BattleResult r);
    void adjust_guardian_indices_after_remove(int owner_id, int removed_index);
    bool try_assign_guardian_at_current(int monster_index);
    void ai_try_assign_guardian();

    void try_wild_capture();
    void start_wild_battle();
    void finish_wild_flee();
    void camp_heal_monster(int index);
    void camp_release_last();
    void grant_random_loot_item(Player& player);
    void ai_wild_choice();
    void ai_camp_choice();
    void ai_arena_choice();
    void enter_turn_end_(const std::wstring& msg);

    float sub_menu_timer_ = 0.f;
    static constexpr float kSubMenuAiDelay = 0.6f;

    bool load_from_save_ = false;
    GameMode initial_game_mode_ = GameMode::PvE;
    int turn_count_ = 0;
    GameMode game_mode_ = GameMode::PvE;

    Board board_;
    Player players_[2];
    BoardRenderer renderer_;
    sf::Font font_;

    // 回合状态
    int current_player_ = 0;       // 0 或 1
    TurnPhase phase_ = TurnPhase::WaitRoll;
    int dice_result_ = 0;
    int steps_remaining_ = 0;
    float move_timer_ = 0.f;
    static constexpr float kMoveInterval = 0.15f;  // 每步动画间隔

    // HUD 信息
    std::wstring message_;

    // AI 延迟计时
    float ai_think_timer_ = 0.f;
    static constexpr float kAiThinkDelay = 0.8f;
    bool ai_guardian_attempted_ = false;

    PendingBattle pending_battle_ = PendingBattle::None;
    int pending_guardian_tile_pos_ = -1;

    Monster wild_encounter_;
    sf::View board_view_;
    sf::View ui_view_;
    sf::Vector2f camera_pos_;
};
