#pragma once

#include "../core/GameState.h"
#include "../entity/Board.h"
#include "../entity/Player.h"
#include "../render/BoardRenderer.h"
#include "../utils/Random.h"

// 回合阶段
enum class TurnPhase {
    WaitRoll,       // 等待掷骰
    Moving,         // 棋子移动动画
    TileAction,     // 格子事件处理
    TurnEnd         // 回合结束，切换玩家
};

class PlayState : public GameState {
public:
    void init() override;
    void cleanup() override {}
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    void roll_dice();
    void next_turn();
    void on_tile_landed();
    int check_game_over() const;  // 返回赢家ID，-1表示游戏继续

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
};
