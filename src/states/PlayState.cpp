#include "PlayState.h"
#include "../core/Game.h"
#include "../system/MonsterSystem.h"
#include "../system/EventSystem.h"
#include "../system/AISystem.h"
#include "BattleState.h"
#include "ShopState.h"
#include "ResultState.h"
#include <cmath>

void PlayState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    board_.init_default();
    players_[0] = Player(kPlayer1Id, "Player1");
    players_[1] = Player(kPlayer2Id, "Player2");

    // 给每个玩家一只初始精灵
    auto starters = MonsterSystem::get().get_starter_choices();
    if (!starters.empty()) {
        players_[0].add_monster(starters[0]);  // 小火龙
        if (starters.size() > 1) {
            players_[1].add_monster(starters[1]);  // 杰尼龟
        } else if (starters.size() > 2) {
            players_[1].add_monster(starters[2]);  // 妙蛙种子
        }
    }

    renderer_.init(board_);

    current_player_ = 0;
    phase_ = TurnPhase::WaitRoll;
    message_ = L"\u73a9\u5bb61 \u7684\u56de\u5408\uff0c\u6309\u7a7a\u683c\u63b7\u9ab0";
}

void PlayState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Space && phase_ == TurnPhase::WaitRoll) {
            roll_dice();
        }
    }
}

void PlayState::roll_dice() {
    dice_result_ = Random::get().roll_dice();
    steps_remaining_ = dice_result_;
    move_timer_ = 0.f;
    phase_ = TurnPhase::Moving;

    message_ = (current_player_ == 0 ? L"\u73a9\u5bb61" : L"\u73a9\u5bb62");
    message_ += L" \u63b7\u51fa\u4e86 " + std::to_wstring(dice_result_) + L" \u70b9";
}

void PlayState::update(float dt) {
    // AI自动掷骰（只在她自己的回合）
    if (phase_ == TurnPhase::WaitRoll) {
        if (current_player_ == 1) {
            ai_think_timer_ += dt;
            if (ai_think_timer_ >= kAiThinkDelay) {
                ai_think_timer_ = 0.f;
                roll_dice();
            }
        } else {
            ai_think_timer_ = 0.f;  // 重置，避免累积
        }
    }

    if (phase_ == TurnPhase::Moving) {
        move_timer_ += dt;
        if (move_timer_ >= kMoveInterval && steps_remaining_ > 0) {
            move_timer_ = 0.f;
            Player& p = players_[current_player_];
            int old_pos = p.position();
            int new_pos = board_.forward(old_pos, 1);
            p.set_position(new_pos);

            if (new_pos == 0 && old_pos != 0) {
                p.add_gold(kLapReward);
                p.add_lap();
            }

            --steps_remaining_;
            if (steps_remaining_ == 0) {
                phase_ = TurnPhase::TileAction;
                on_tile_landed();
            }
        }
    }

    if (phase_ == TurnPhase::TurnEnd) {
        move_timer_ += dt;
        if (move_timer_ >= 0.8f) {
            next_turn();
        }
    }
}

void PlayState::on_tile_landed() {
    Player& p = players_[current_player_];
    const Tile& tile = board_.tile_at(p.position());

    switch (tile.type()) {
        case TileType::Start:
            message_ = L"\u56de\u5230\u8d77\u70b9\uff0c\u83b7\u5f97 " + std::to_wstring(kLapReward) + L" \u91d1\u5e01";
            break;
        case TileType::Empty:
            if (!tile.has_owner()) {
                // AI决策是否购买
                bool should_buy = (current_player_ == 0) || AISystem::get().should_buy_property(p, tile);
                if (should_buy && p.spend_gold(tile.price())) {
                    board_.tile_at(p.position()).set_owner(p.id());
                    p.add_property(p.position());
                    message_ = L"\u8d2d\u4e70\u4e86\u5730\u76ae\uff01\u82b1\u8d39 " + std::to_wstring(tile.price()) + L"G";
                } else if (!should_buy) {
                    message_ = L"\u7a7a\u5730\uff0c\u8df3\u8fc7\u8d2d\u4e70";
                } else {
                    message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff0c\u65e0\u6cd5\u8d2d\u4e70";
                }
            } else if (tile.owner_id() != p.id()) {
                int toll = tile.toll();
                if (p.gold() >= toll) {
                    p.spend_gold(toll);
                    players_[tile.owner_id()].add_gold(toll);
                    message_ = L"\u7ecf\u8fc7\u5bf9\u65b9\u5730\u76ae\uff0c\u4ea4\u8fc7\u8def\u8d39 " + std::to_wstring(toll) + L"G";
                } else {
                    message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff0c\u65e0\u6cd5\u4ea4\u8fc7\u8def\u8d39\uff01";
                }
            } else {
                if (tile.can_upgrade()) {
                    int cost = tile.upgrade_cost();
                    bool should_upgrade = (current_player_ == 0) || AISystem::get().should_upgrade_property(p, tile);
                    if (should_upgrade && p.spend_gold(cost)) {
                        board_.tile_at(p.position()).upgrade();
                        message_ = L"\u5347\u7ea7\u5730\u76ae\uff01\u82b1\u8d39 " + std::to_wstring(cost) + L"G";
                    } else if (!should_upgrade) {
                        message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u8df3\u8fc7\u5347\u7ea7";
                    } else {
                        message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u91d1\u5e01\u4e0d\u8db3\u5347\u7ea7";
                    }
                } else {
                    message_ = L"\u81ea\u5df1\u7684\u5730\u76ae\uff0c\u5df2\u6ee1\u7ea7";
                }
            }
            break;
        case TileType::Wild: {
            // 生成野生精灵并进入战斗
            Monster wild = MonsterSystem::get().generate_wild_monster();
            if (!p.monsters().empty()) {
                Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
                bool ai = (current_player_ == 1);
                push_state(Game::get().state_manager(),
                          new BattleState(player_mon, wild, BattleType::Wild, ai));
                // 战斗结束后再处理回合结束
                phase_ = TurnPhase::TurnEnd;
                move_timer_ = 0.f;
                return;
            } else {
                message_ = L"\u6ca1\u6709\u7cbe\u7075\uff0c\u65e0\u6cd5\u8bdb\u6597\uff01";
            }
            break;
        }
        case TileType::Shop:
            push_state(Game::get().state_manager(), new ShopState(players_[current_player_]));
            phase_ = TurnPhase::TurnEnd;
            move_timer_ = 0.f;
            return;
        case TileType::Event: {
            EventSystem::get().load_from_json("assets/data/events.json");
            RandomEvent ev = EventSystem::get().trigger_event(players_[current_player_]);
            message_ = EventSystem::get().get_event_description(ev);
            break;
        }
        case TileType::Camp: {
            // 营地：恢复所有精灵HP
            for (auto& mon : players_[current_player_].monsters()) {
                if (mon.is_alive()) {
                    mon.heal(mon.max_hp());
                }
            }
            message_ = L"\u5230\u8fbe\u8425\u5730\uff0c\u7cbe\u7075\u5df2\u6062\u590d\uff01";
            break;
        }
        case TileType::Battle: {
            // 对战格：强制PvP
            Player& opponent = players_[1 - current_player_];
            if (!p.monsters().empty() && !opponent.monsters().empty()) {
                Monster& player_mon = const_cast<Monster&>(p.monsters()[0]);
                Monster& opponent_mon = const_cast<Monster&>(opponent.monsters()[0]);
                bool ai = (current_player_ == 1);
                push_state(Game::get().state_manager(),
                          new BattleState(player_mon, opponent_mon, BattleType::PvP, ai));
                phase_ = TurnPhase::TurnEnd;
                move_timer_ = 0.f;
                return;
            } else {
                message_ = L"双方需要有精灵才能对战！";
            }
            break;
        }
    }

    move_timer_ = 0.f;
    phase_ = TurnPhase::TurnEnd;
}

void PlayState::next_turn() {
    // 检查游戏是否结束
    int winner = check_game_over();
    if (winner >= 0) {
        GameResult result = (winner == 0) ? GameResult::Player1Win : GameResult::Player2Win;
        push_state(Game::get().state_manager(), new ResultState(result, winner));
        return;
    }

    current_player_ = 1 - current_player_;
    phase_ = TurnPhase::WaitRoll;
    move_timer_ = 0.f;

    message_ = (current_player_ == 0 ? L"\u73a9\u5bb61" : L"\u73a9\u5bb62");
    message_ += L" \u7684\u56de\u5408\uff0c\u6309\u7a7a\u683c\u63b7\u9ab0";
}

int PlayState::check_game_over() const {
    // 检查玩家1
    bool p1_alive = players_[0].has_alive_monsters();
    bool p1_bankrupt = (players_[0].gold() == 0) && (players_[0].total_properties() == 0);

    // 检查玩家2
    bool p2_alive = players_[1].has_alive_monsters();
    bool p2_bankrupt = (players_[1].gold() == 0) && (players_[1].total_properties() == 0);

    // 精灵全灭或破产则失败
    if (!p1_alive || p1_bankrupt) return 1;  // 玩家2获胜
    if (!p2_alive || p2_bankrupt) return 0;  // 玩家1获胜

    // 检查是否有人达到5圈（简单胜利条件）
    if (players_[0].laps_completed() >= 5) return 0;
    if (players_[1].laps_completed() >= 5) return 1;

    return -1;  // 游戏继续
}

// ── 渲染辅助 ──────────────────────────────────────────

static void draw_panel(sf::RenderWindow& window, float x, float y, float w, float h,
                       sf::Color fill = sf::Color(25, 25, 50, 200)) {
    // 阴影
    sf::RectangleShape shadow({w, h});
    shadow.setPosition(x + 2.f, y + 2.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 50));
    window.draw(shadow);

    // 面板
    sf::RectangleShape panel({w, h});
    panel.setPosition(x, y);
    panel.setFillColor(fill);
    panel.setOutlineThickness(1.f);
    panel.setOutlineColor(sf::Color(60, 60, 120, 100));
    window.draw(panel);
}

static void draw_label(sf::RenderWindow& window, sf::Font& font,
                       const std::wstring& text, float x, float y,
                       int size, sf::Color color) {
    sf::Text t;
    t.setFont(font);
    t.setString(text);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setPosition(x, y);
    window.draw(t);
}

static void draw_bar(sf::RenderWindow& window, float x, float y, float w, float h,
                     float ratio, sf::Color fill, sf::Color bg = sf::Color(40, 40, 60)) {
    sf::RectangleShape back({w, h});
    back.setPosition(x, y);
    back.setFillColor(bg);
    window.draw(back);

    sf::RectangleShape bar({w * ratio, h});
    bar.setPosition(x, y);
    bar.setFillColor(fill);
    window.draw(bar);
}

void PlayState::render(sf::RenderWindow& window) {
    // 棋盘
    renderer_.render(window, board_, players_[0], players_[1]);

    // ── 右侧面板区域 ──
    float px = 560.f;
    float pw = 380.f;

    // 回合指示
    draw_panel(window, px, 15.f, pw, 30.f, sf::Color(30, 30, 60, 220));
    std::wstring turn_text = L"\u7b2c " + std::to_wstring(players_[0].laps_completed() + players_[1].laps_completed() + 1) + L" \u56de\u5408";
    draw_label(window, font_, turn_text, px + 10.f, 18.f, 16, sf::Color(160, 160, 200));

    std::wstring cur = (current_player_ == 0) ? L"\u25c6 \u73a9\u5bb61" : L"\u25c6 \u73a9\u5bb62";
    draw_label(window, font_, cur, px + pw - 110.f, 18.f, 16,
               current_player_ == 0 ? sf::Color(80, 180, 255) : sf::Color(255, 140, 80));

    // 玩家1面板
    {
        float py = 60.f;
        sf::Color accent(60, 140, 230);
        draw_panel(window, px, py, pw, 130.f);

        // 色条
        sf::RectangleShape bar({4.f, 130.f});
        bar.setPosition(px, py);
        bar.setFillColor(accent);
        window.draw(bar);

        // 当前玩家高亮
        if (current_player_ == 0) {
            sf::RectangleShape highlight({pw, 130.f});
            highlight.setPosition(px, py);
            highlight.setFillColor(sf::Color(60, 140, 230, 15));
            window.draw(highlight);
        }

        draw_label(window, font_, L"\u73a9\u5bb61", px + 14.f, py + 6.f, 18, accent);
        draw_label(window, font_, L"\u91d1\u5e01", px + 14.f, py + 36.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[0].gold()) + L" G",
                   px + 60.f, py + 34.f, 15, sf::Color(255, 220, 100));

        draw_label(window, font_, L"\u5730\u76ae", px + 14.f, py + 62.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[0].total_properties()) + L" \u5757",
                   px + 60.f, py + 60.f, 15, sf::Color(200, 200, 220));

        draw_label(window, font_, L"\u7cbe\u7075", px + 14.f, py + 88.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[0].monsters().size()) + L" / " + std::to_wstring(kMaxMonsters),
                   px + 60.f, py + 86.f, 15, sf::Color(200, 200, 220));

        // 金币条
        float gold_ratio = static_cast<float>(players_[0].gold()) / static_cast<float>(kInitGold * 3);
        gold_ratio = std::min(1.f, gold_ratio);
        draw_bar(window, px + 14.f, py + 115.f, pw - 28.f, 4.f, gold_ratio, accent);
    }

    // 玩家2面板
    {
        float py = 205.f;
        sf::Color accent(230, 110, 60);
        draw_panel(window, px, py, pw, 130.f);

        sf::RectangleShape bar({4.f, 130.f});
        bar.setPosition(px, py);
        bar.setFillColor(accent);
        window.draw(bar);

        if (current_player_ == 1) {
            sf::RectangleShape highlight({pw, 130.f});
            highlight.setPosition(px, py);
            highlight.setFillColor(sf::Color(230, 110, 60, 15));
            window.draw(highlight);
        }

        draw_label(window, font_, L"\u73a9\u5bb62", px + 14.f, py + 6.f, 18, accent);
        draw_label(window, font_, L"\u91d1\u5e01", px + 14.f, py + 36.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[1].gold()) + L" G",
                   px + 60.f, py + 34.f, 15, sf::Color(255, 220, 100));

        draw_label(window, font_, L"\u5730\u76ae", px + 14.f, py + 62.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[1].total_properties()) + L" \u5757",
                   px + 60.f, py + 60.f, 15, sf::Color(200, 200, 220));

        draw_label(window, font_, L"\u7cbe\u7075", px + 14.f, py + 88.f, 13, sf::Color(140, 140, 170));
        draw_label(window, font_, std::to_wstring(players_[1].monsters().size()) + L" / " + std::to_wstring(kMaxMonsters),
                   px + 60.f, py + 86.f, 15, sf::Color(200, 200, 220));

        float gold_ratio = static_cast<float>(players_[1].gold()) / static_cast<float>(kInitGold * 3);
        gold_ratio = std::min(1.f, gold_ratio);
        draw_bar(window, px + 14.f, py + 115.f, pw - 28.f, 4.f, gold_ratio, accent);
    }

    // 骰子显示区
    {
        float dy = 350.f;
        draw_panel(window, px, dy, pw, 80.f);

        draw_label(window, font_, L"\u63b7\u9ab0", px + 14.f, dy + 8.f, 14, sf::Color(140, 140, 170));

        if (dice_result_ > 0) {
            // 骰子数字大字
            sf::Text dice;
            dice.setFont(font_);
            dice.setString(std::to_wstring(dice_result_));
            dice.setCharacterSize(40);
            dice.setFillColor(sf::Color(255, 220, 100));
            dice.setStyle(sf::Text::Bold);
            auto db = dice.getLocalBounds();
            dice.setOrigin(db.left + db.width / 2.f, db.top + db.height / 2.f);
            dice.setPosition(px + pw / 2.f, dy + 48.f);
            window.draw(dice);
        } else {
            draw_label(window, font_, L"\u6309\u7a7a\u683c\u63b7\u9ab0", px + pw / 2.f - 55.f, dy + 35.f,
                       16, sf::Color(100, 100, 140));
        }
    }

    // 消息栏
    {
        float my = 445.f;
        draw_panel(window, px, my, pw, 50.f, sf::Color(20, 20, 45, 230));

        sf::Text msg;
        msg.setFont(font_);
        msg.setString(message_);
        msg.setCharacterSize(15);
        msg.setFillColor(sf::Color(255, 230, 120));
        msg.setPosition(px + 14.f, my + 14.f);
        window.draw(msg);
    }

    // 操作提示
    {
        float hy = 510.f;
        draw_label(window, font_, L"\u7a7a\u683c: \u63b7\u9ab0    ESC: \u9000\u51fa",
                   px + 14.f, hy, 12, sf::Color(80, 80, 120));
    }
}
