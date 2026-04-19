#include "BattleState.h"
#include "../core/Game.h"
#include "../system/BattleSystem.h"
#include "../ui/RetroUI.h"
#include <cmath>

namespace {
std::wstring utf8_to_wstring(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end()).toWideString();
}
}

BattleExitSnapshot BattleState::s_last_exit_{};

BattleExitSnapshot BattleState::consume_exit_snapshot() {
    BattleExitSnapshot out = s_last_exit_;
    s_last_exit_.valid = false;
    return out;
}

BattleState::BattleState(Monster& player_monster, Monster& enemy_monster, BattleType type, bool ai_controlled)
    : player_monster_(player_monster)
    , enemy_monster_(enemy_monster)
    , battle_type_(type)
    , battle_sys_(BattleSystem::get())
    , ai_controlled_(ai_controlled) {}

void BattleState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    // 尝试加载对战背景贴图
    bg_texture_.loadFromFile("assets/textures/battle_bg.png");

    // 尝试加载精灵贴图 (优先找背影，找不到找正面)
    if (!player_mon_tex_.loadFromFile("assets/textures/monster_back_" + std::to_string(player_monster_.id()) + ".png")) {
        player_mon_tex_.loadFromFile("assets/textures/monster_front_" + std::to_string(player_monster_.id()) + ".png");
    }
    enemy_mon_tex_.loadFromFile("assets/textures/monster_front_" + std::to_string(enemy_monster_.id()) + ".png");

    battle_sys_.start_battle(player_monster_, enemy_monster_, battle_type_);

    player_hp_before_ = player_monster_.current_hp();
    enemy_hp_before_ = enemy_monster_.current_hp();

    battle_message_ = L"Wild monster! Choose action.";
    phase_ = Phase::SelectAction;

    create_action_buttons();
    create_skill_buttons();

    ui_view_.setSize(1440.f, 960.f);
    ui_view_.setCenter(720.f, 480.f);
}

void BattleState::cleanup() {
    s_last_exit_.valid = true;
    s_last_exit_.result = battle_sys_.result().result;
    s_last_exit_.type = battle_type_;
    s_last_exit_.gold_reward = battle_sys_.result().gold_reward;
}

void BattleState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y), ui_view_);

        for (auto& btn : action_buttons_) {
            btn.hover = contains(btn, mouse);
        }
        for (auto& btn : skill_buttons_) {
            btn.hover = contains(btn, mouse);
        }
        back_button_.hover = back_button_.shape.getGlobalBounds().contains(mouse);
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y), ui_view_);

        if (phase_ == Phase::SelectAction) {
            for (size_t i = 0; i < action_buttons_.size(); ++i) {
                if (contains(action_buttons_[i], mouse)) {
                    if (i == 0) {
                        phase_ = Phase::SelectSkill;
                        return; // 防止同一帧触发技能点击
                    } else if (i == 1) {
                        TurnAction action;
                        action.type = TurnAction::Capture;
                        action.ball_bonus = 0;
                        battle_sys_.execute_action(action);
                        battle_message_ = get_message_text(battle_sys_.result().message_id);

                        if (battle_sys_.result().result != BattleResult::Ongoing) {
                            phase_ = Phase::Result;
                            result_timer_ = 0.f;
                        } else {
                            phase_ = Phase::Executing;
                            result_timer_ = 0.f;
                        }
                        return;
                    } else if (i == 2) {
                        TurnAction action;
                        action.type = TurnAction::Escape;
                        battle_sys_.execute_action(action);
                        battle_message_ = get_message_text(battle_sys_.result().message_id);

                        if (battle_sys_.result().result != BattleResult::Ongoing) {
                            phase_ = Phase::Result;
                            result_timer_ = 0.f;
                        } else {
                            phase_ = Phase::Executing;
                            result_timer_ = 0.f;
                        }
                        return;
                    }
                    break;
                }
            }
        } else if (phase_ == Phase::SelectSkill) {
            for (size_t i = 0; i < skill_buttons_.size(); ++i) {
                if (contains(skill_buttons_[i], mouse)) {
                    selected_skill_ = static_cast<int>(i);
                    TurnAction action;
                    action.type = TurnAction::UseSkill;
                    action.skill_index = selected_skill_;
                    battle_sys_.execute_action(action);
                    battle_message_ = get_message_text(battle_sys_.result().message_id);

                    if (battle_sys_.result().result != BattleResult::Ongoing) {
                        phase_ = Phase::Result;
                        result_timer_ = 0.f;
                    } else {
                        phase_ = Phase::Executing;
                        result_timer_ = 0.f;
                    }
                    return;
                }
            }

            if (contains(back_button_, mouse)) {
                phase_ = Phase::SelectAction;
                return;
            }
        } else if (phase_ == Phase::Result) {
            BattleResult r = battle_sys_.result().result;
            if (r == BattleResult::Captured) {
                captured_monster_ = enemy_monster_;
            }
            pop_state(Game::get().state_manager());
            return;
        }
    }
}

void BattleState::update(float dt) {
    if (phase_ == Phase::Executing) {
        result_timer_ += dt;
        if (result_timer_ >= 0.5f) {
            phase_ = Phase::SelectAction;
            player_hp_before_ = player_monster_.current_hp();
            enemy_hp_before_ = enemy_monster_.current_hp();
            battle_message_ = get_message_text(battle_sys_.result().message_id);
        }
    }

    if (phase_ == Phase::Result) {
        result_timer_ += dt;
    }
}

std::wstring BattleState::get_message_text(int id) const {
    switch (id) {
        case 1: return L"Escaped!";
        case 2: return L"Escape failed!";
        case 3: return L"Cannot escape here!";
        case 4: return L"Captured!";
        case 5: return L"Capture failed!";
        case 6: return L"Cannot capture here!";
        case 7: return L"No PP!";
        case 8: return L"Miss!";
        case 9: return L"Victory!";
        case 10: return L"Defeated!";
        default: return L"...";
    }
}

void BattleState::render(sf::RenderWindow& window) {
    window.setView(ui_view_);

    // 纯色背景或贴图背景
    if (bg_texture_.getSize().x > 0) {
        sf::Sprite bg(bg_texture_);
        bg.setScale(1440.f / bg_texture_.getSize().x, 960.f / bg_texture_.getSize().y);
        window.draw(bg);
    } else {
        sf::RectangleShape bg({1440.f, 960.f});
        bg.setFillColor(sf::Color(248, 248, 240));
        window.draw(bg);
    }

    // 绘制精灵贴图
    if (enemy_mon_tex_.getSize().x > 0) {
        sf::Sprite spr(enemy_mon_tex_);
        spr.setScale(300.f / enemy_mon_tex_.getSize().x, 300.f / enemy_mon_tex_.getSize().y);
        spr.setPosition(900.f, 150.f);
        window.draw(spr);
    } else {
        sf::RectangleShape spr({300.f, 300.f});
        spr.setFillColor(sf::Color(255, 140, 80));
        spr.setPosition(900.f, 150.f);
        window.draw(spr);
    }

    if (player_mon_tex_.getSize().x > 0) {
        sf::Sprite spr(player_mon_tex_);
        spr.setScale(350.f / player_mon_tex_.getSize().x, 350.f / player_mon_tex_.getSize().y);
        spr.setPosition(200.f, 400.f);
        window.draw(spr);
    } else {
        sf::RectangleShape spr({350.f, 350.f});
        spr.setFillColor(sf::Color(80, 180, 255));
        spr.setPosition(200.f, 400.f);
        window.draw(spr);
    }

    // 绘制双方精灵面板 (经典位置)
    draw_status_box(window, enemy_monster_, 100.f, 100.f, false);
    draw_status_box(window, player_monster_, 850.f, 500.f, true);

    // 消息框 (底部通栏)
    RetroUI::draw_box(window, 20.f, 700.f, 1400.f, 240.f, sf::Color::White, sf::Color(40, 40, 40), 6.f);
    
    // 如果不在选择阶段，或者在选择技能阶段，消息框显示文字
    if (phase_ != Phase::SelectAction) {
        RetroUI::draw_text(window, battle_message_, 60.f, 740.f, 32, sf::Color(40, 40, 40));
    } else {
        RetroUI::draw_text(window, L"要做什么？", 60.f, 740.f, 32, sf::Color(40, 40, 40));
    }

    // 如果 AI 正在思考，显示提示
    if (ai_controlled_ && phase_ == Phase::SelectAction) {
        RetroUI::draw_text(window, L"AI 思考中…", 60.f, 800.f, 24, sf::Color(100, 100, 100));
    } else if (ai_controlled_ && phase_ == Phase::SelectSkill) {
        RetroUI::draw_text(window, L"AI 正在选择技能…", 60.f, 800.f, 24, sf::Color(100, 100, 100));
    }

    if (phase_ == Phase::SelectAction) {
        // 绘制右下角的 2x2 菜单框
        RetroUI::draw_box(window, 900.f, 700.f, 520.f, 240.f, sf::Color::White, sf::Color(40, 40, 40), 6.f);
        for (auto& btn : action_buttons_) {
            RetroUI::draw_text(window, btn.text.getString().toWideString(),
                               btn.shape.getPosition().x + 40.f, btn.shape.getPosition().y + 15.f,
                               32, sf::Color(40, 40, 40));
            if (btn.hover) {
                RetroUI::draw_text(window, L"►", btn.shape.getPosition().x + 5.f, btn.shape.getPosition().y + 15.f, 32, sf::Color(40, 40, 40));
            }
        }
    }

    if (phase_ == Phase::SelectSkill) {
        RetroUI::draw_box(window, 900.f, 700.f, 520.f, 240.f, sf::Color::White, sf::Color(40, 40, 40), 6.f);
        for (auto& btn : skill_buttons_) {
            RetroUI::draw_text(window, btn.text.getString().toWideString(),
                               btn.shape.getPosition().x + 40.f, btn.shape.getPosition().y + 15.f,
                               24, sf::Color(40, 40, 40));
            if (btn.hover) {
                RetroUI::draw_text(window, L"►", btn.shape.getPosition().x + 5.f, btn.shape.getPosition().y + 15.f, 24, sf::Color(40, 40, 40));
            }
        }

        RetroUI::draw_text(window, back_button_.text.getString().toWideString(),
                           back_button_.shape.getPosition().x + 40.f, back_button_.shape.getPosition().y + 15.f,
                           24, sf::Color(40, 40, 40));
        if (back_button_.hover) {
            RetroUI::draw_text(window, L"►", back_button_.shape.getPosition().x + 5.f, back_button_.shape.getPosition().y + 15.f, 24, sf::Color(40, 40, 40));
        }
    }

    if (phase_ == Phase::Executing) {
        RetroUI::draw_text(window, L"▼", 1350.f, 880.f, 24, sf::Color(100, 100, 100));
    }

    if (phase_ == Phase::Result) {
        RetroUI::draw_text(window, L"▼ 点击继续", 1250.f, 880.f, 24, sf::Color(100, 100, 100));
    }
}

void BattleState::draw_status_box(sf::RenderWindow& window, const Monster& mon,
                                  float x, float y, bool is_player_mon) {
    float pw = 450.f;
    float ph = is_player_mon ? 160.f : 120.f;

    RetroUI::draw_box(window, x, y, pw, ph, sf::Color::White, sf::Color(40, 40, 40), 6.f);

    // 名称和等级
    std::wstring name_str = utf8_to_wstring(mon.name());
    if (name_str.empty()) name_str = L"P" + std::to_wstring(mon.id());
    RetroUI::draw_text(window, name_str, x + 20.f, y + 15.f, 28, sf::Color(40, 40, 40));
    
    std::wstring lv_str = L"Lv" + std::to_wstring(mon.level());
    RetroUI::draw_text(window, lv_str, x + 300.f, y + 15.f, 24, sf::Color(40, 40, 40));

    // HP标签
    RetroUI::draw_text(window, L"HP", x + 30.f, y + 65.f, 20, sf::Color(200, 180, 40), true);

    // HP血条
    RetroUI::draw_hp_bar(window, x + 80.f, y + 70.f, 320.f, 20.f, mon.current_hp(), mon.max_hp());

    // 玩家精灵额外显示具体数字
    if (is_player_mon) {
        std::wstring hp_str = std::to_wstring(mon.current_hp()) + L" / " + std::to_wstring(mon.max_hp());
        RetroUI::draw_text(window, hp_str, x + 250.f, y + 105.f, 24, sf::Color(40, 40, 40));
    }
}

void BattleState::create_action_buttons() {
    action_buttons_.clear();
    const wchar_t* labels[] = { L"战斗", L"捕捉", L"逃跑", L"物品" };

    for (int i = 0; i < 4; ++i) {
        Button btn;
        btn.shape.setSize({200.f, 60.f});
        // 2x2 网格，放在右下角
        float bx = 940.f + (i % 2) * 220.f;
        float by = 740.f + (i / 2) * 80.f;
        btn.shape.setPosition(bx, by);
        btn.shape.setFillColor(sf::Color::Transparent);

        btn.text.setFont(font_);
        btn.text.setString(labels[i]);
        action_buttons_.push_back(btn);
    }
}

void BattleState::create_skill_buttons() {
    skill_buttons_.clear();

    const auto& skills = player_monster_.skills();
    for (size_t i = 0; i < skills.size() && i < 4; ++i) {
        Button btn;
        btn.shape.setSize({380.f, 60.f});
        // 2x2 网格
        float bx = 940.f + (i % 2) * 220.f; // 稍微挤一点
        float by = 740.f + (i / 2) * 80.f;
        btn.shape.setPosition(bx, by);
        btn.shape.setFillColor(sf::Color::Transparent);

        btn.text.setFont(font_);
        std::wstring label = utf8_to_wstring(skills[i].name);
        btn.text.setString(label);
        skill_buttons_.push_back(btn);
    }

    back_button_.shape.setSize({120.f, 60.f});
    back_button_.shape.setPosition(1280.f, 860.f);
    back_button_.shape.setFillColor(sf::Color::Transparent);
    back_button_.text.setFont(font_);
    back_button_.text.setString(L"返回");
    back_button_.hover = false;
}

bool BattleState::contains(const Button& btn, const sf::Vector2f& point) const {
    return btn.shape.getGlobalBounds().contains(point);
}
