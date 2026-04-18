#pragma once

#include "../core/GameState.h"
#include <string>

enum class GameResult { Player1Win, Player2Win, Draw };

class ResultState : public GameState {
public:
    ResultState(GameResult result, int winner_id = -1);

    void init() override;
    void cleanup() override;
    void handle_event(const sf::Event& event) override;
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

private:
    GameResult result_;
    int winner_id_;
    sf::Font font_;
};
