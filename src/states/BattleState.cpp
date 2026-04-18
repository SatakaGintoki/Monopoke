#include "BattleState.h"
#include "../core/Game.h"
#include "../system/BattleSystem.h"
#include "../system/AISystem.h"
#include <cmath>

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

    battle_sys_.start_battle(player_monster_, enemy_monster_, battle_type_);

    player_hp_before_ = player_monster_.current_hp();
    enemy_hp_before_ = enemy_monster_.current_hp();

    battle_message_ = L"Wild monster! Choose action.";
    phase_ = Phase::SelectAction;

    create_action_buttons();
    create_skill_buttons();
}

void BattleState::cleanup() {}

void BattleState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse(static_cast<float>(event.mouseMove.x),
                           static_cast<float>(event.mouseMove.y));

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
        sf::Vector2f mouse(static_cast<float>(event.mouseButton.x),
                           static_cast<float>(event.mouseButton.y));

        if (phase_ == Phase::SelectAction) {
            for (size_t i = 0; i < action_buttons_.size(); ++i) {
                if (contains(action_buttons_[i], mouse)) {
                    if (i == 0) {
                        phase_ = Phase::SelectSkill;
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
                    }
                    break;
                }
            }
        }

        if (phase_ == Phase::SelectSkill) {
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
                    break;
                }
            }

            if (contains(back_button_, mouse)) {
                phase_ = Phase::SelectAction;
            }
        }

        if (phase_ == Phase::Result) {
            BattleResult r = battle_sys_.result().result;
            if (r == BattleResult::Captured) {
                captured_monster_ = enemy_monster_;
            }
            pop_state(Game::get().state_manager());
        }
    }
}

void BattleState::update(float dt) {
    // AI自动操作
    if (ai_controlled_) {
        if (phase_ == Phase::SelectAction || phase_ == Phase::SelectSkill) {
            ai_timer_ += dt;
            if (ai_timer_ >= kAiActionDelay) {
                ai_timer_ = 0.f;
                // AI选择技能
                int skill_idx = AISystem::get().choose_skill(player_monster_, enemy_monster_);
                if (skill_idx < 0) skill_idx = 0;

                TurnAction action;
                action.type = TurnAction::UseSkill;
                action.skill_index = skill_idx;
                battle_sys_.execute_action(action);
                battle_message_ = get_message_text(battle_sys_.result().message_id);

                if (battle_sys_.result().result != BattleResult::Ongoing) {
                    phase_ = Phase::Result;
                    result_timer_ = 0.f;
                } else {
                    phase_ = Phase::Executing;
                    result_timer_ = 0.f;
                }
            }
        }

        if (phase_ == Phase::Result) {
            result_timer_ += dt;
            if (result_timer_ >= 1.0f) {
                pop_state(Game::get().state_manager());
                return;
            }
        }
    }

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
    window.clear(sf::Color(15, 15, 35));

    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape band({960.f, 80.f});
        band.setPosition(0.f, i * 80.f);
        sf::Uint8 r = static_cast<sf::Uint8>(15 + i * 2);
        sf::Uint8 b = static_cast<sf::Uint8>(30 + i * 5);
        band.setFillColor(sf::Color(r, r, b));
        window.draw(band);
    }

    {
        sf::Text title;
        title.setFont(font_);
        title.setString(L"BATTLE!");
        title.setCharacterSize(32);
        title.setFillColor(sf::Color(255, 220, 100));
        auto tb = title.getLocalBounds();
        title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
        title.setPosition(480.f, 20.f);
        window.draw(title);
    }

    draw_monster_panel(window, enemy_monster_, 40.f, 70.f, true, false);
    draw_monster_panel(window, player_monster_, 520.f, 280.f, false, true);

    {
        sf::Text msg;
        msg.setFont(font_);
        msg.setString(battle_message_);
        msg.setCharacterSize(18);
        msg.setFillColor(sf::Color(255, 230, 120));
        auto mb = msg.getLocalBounds();
        msg.setOrigin(mb.left + mb.width / 2.f, mb.top + mb.height / 2.f);
        msg.setPosition(480.f, 250.f);
        window.draw(msg);
    }

    if (phase_ == Phase::SelectAction) {
        for (auto& btn : action_buttons_) {
            sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + btn.hover * 135);
            btn.shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));
            window.draw(btn.shape);
            window.draw(btn.text);
        }
    }

    if (phase_ == Phase::SelectSkill) {
        for (auto& btn : skill_buttons_) {
            sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + btn.hover * 135);
            btn.shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));
            window.draw(btn.shape);
            window.draw(btn.text);
        }

        if (back_button_.hover) {
            back_button_.shape.setOutlineColor(sf::Color(100, 120, 200, 200));
        }
        window.draw(back_button_.shape);
        window.draw(back_button_.text);
    }

    if (phase_ == Phase::Executing) {
        sf::Text hint;
        hint.setFont(font_);
        hint.setString("Processing...");
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(150, 150, 180));
        auto hb = hint.getLocalBounds();
        hint.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
        hint.setPosition(480.f, 500.f);
        window.draw(hint);
    }

    if (phase_ == Phase::Result) {
        sf::Text hint;
        hint.setFont(font_);
        hint.setString("Click to continue");
        hint.setCharacterSize(16);
        hint.setFillColor(sf::Color(150, 150, 180));
        auto hb = hint.getLocalBounds();
        hint.setOrigin(hb.left + hb.width / 2.f, hb.top + hb.height / 2.f);
        hint.setPosition(480.f, 520.f);
        window.draw(hint);
    }
}

void BattleState::draw_monster_panel(sf::RenderWindow& window, const Monster& mon,
                                     float x, float y, bool is_left, bool is_player_mon) {
    float pw = 400.f;
    float ph = 200.f;

    sf::RectangleShape panel({pw, ph});
    panel.setPosition(x, y);
    panel.setFillColor(sf::Color(20, 20, 45, 230));
    panel.setOutlineThickness(1.5f);
    panel.setOutlineColor(sf::Color(60, 60, 120, 150));
    window.draw(panel);

    // 名称和等级
    sf::Text name;
    name.setFont(font_);
    std::wstring name_str = L"P" + std::to_wstring(mon.id()) + L" Lv" + std::to_wstring(mon.level());
    name.setString(name_str);
    name.setCharacterSize(18);
    name.setFillColor(is_player_mon ? sf::Color(80, 180, 255) : sf::Color(255, 140, 80));
    name.setPosition(x + 15.f, y + 10.f);
    window.draw(name);

    // HP标签
    sf::Text hp_label;
    hp_label.setFont(font_);
    hp_label.setString(L"HP");
    hp_label.setCharacterSize(13);
    hp_label.setFillColor(sf::Color(140, 140, 170));
    hp_label.setPosition(x + 15.f, y + 55.f);
    window.draw(hp_label);

    // HP数值
    sf::Text hp_val;
    hp_val.setFont(font_);
    hp_val.setString(std::to_wstring(mon.current_hp()) + L" / " + std::to_wstring(mon.max_hp()));
    hp_val.setCharacterSize(14);
    hp_val.setFillColor(sf::Color(255, 220, 100));
    hp_val.setPosition(x + 50.f, y + 53.f);
    window.draw(hp_val);

    // HP血条
    float hp_ratio = static_cast<float>(mon.current_hp()) / static_cast<float>(mon.max_hp());
    sf::Color hp_color = (hp_ratio > 0.5f) ? sf::Color(80, 200, 80)
                          : (hp_ratio > 0.25f) ? sf::Color(200, 200, 60)
                          : sf::Color(200, 60, 60);

    sf::RectangleShape hp_bg({300.f, 12.f});
    hp_bg.setPosition(x + 15.f, y + 78.f);
    hp_bg.setFillColor(sf::Color(40, 40, 60));
    window.draw(hp_bg);

    sf::RectangleShape hp_bar({300.f * hp_ratio, 12.f});
    hp_bar.setPosition(x + 15.f, y + 78.f);
    hp_bar.setFillColor(hp_color);
    window.draw(hp_bar);

    // 能力值
    sf::Text stats;
    stats.setFont(font_);
    std::wstring stat_str = L"ATK:" + std::to_wstring(mon.atk()) + L" DEF:" + std::to_wstring(mon.def()) + L" SPD:" + std::to_wstring(mon.spd());
    stats.setString(stat_str);
    stats.setCharacterSize(12);
    stats.setFillColor(sf::Color(140, 140, 180));
    stats.setPosition(x + 15.f, y + 100.f);
    window.draw(stats);

    // 技能列表
    for (size_t i = 0; i < mon.skills().size() && i < 3; ++i) {
        const auto& skill = mon.skills()[i];
        sf::Text sk;
        sk.setFont(font_);
        std::wstring sk_name = L"Skill" + std::to_wstring(i + 1);
        std::wstring pp_str = L" PP:" + std::to_wstring(skill.pp) + L"/" + std::to_wstring(skill.max_pp);
        sk.setString(sk_name + pp_str);
        sk.setCharacterSize(11);
        sf::Color sk_color = skill.pp > 0 ? sf::Color(200, 200, 220) : sf::Color(100, 100, 120);
        sk.setFillColor(sk_color);
        sk.setPosition(x + 15.f + i * 120.f, y + 165.f);
        window.draw(sk);
    }
}

void BattleState::create_action_buttons() {
    action_buttons_.clear();
    const wchar_t* labels[] = { L"Skill", L"Capture", L"Escape" };

    for (int i = 0; i < 3; ++i) {
        Button btn;
        btn.shape.setSize({150.f, 45.f});
        btn.shape.setPosition(300.f + i * 170.f, 420.f);
        btn.shape.setFillColor(sf::Color(35, 35, 65));
        btn.shape.setOutlineThickness(1.5f);
        btn.shape.setOutlineColor(sf::Color(80, 80, 140, 120));

        btn.text.setFont(font_);
        btn.text.setString(labels[i]);
        btn.text.setCharacterSize(18);
        btn.text.setFillColor(sf::Color(200, 200, 230));
        auto lb = btn.text.getLocalBounds();
        btn.text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        btn.text.setPosition(300.f + i * 170.f + 75.f, 442.5f);

        action_buttons_.push_back(btn);
    }
}

void BattleState::create_skill_buttons() {
    skill_buttons_.clear();

    const auto& skills = player_monster_.skills();
    for (size_t i = 0; i < skills.size() && i < 4; ++i) {
        Button btn;
        btn.shape.setSize({200.f, 40.f});
        float bx = (i < 2) ? 80.f + i * 220.f : 280.f + (i - 2) * 220.f;
        float by = (i < 2) ? 380.f : 440.f;
        btn.shape.setPosition(bx, by);
        btn.shape.setFillColor(sf::Color(35, 35, 65));
        btn.shape.setOutlineThickness(1.5f);
        btn.shape.setOutlineColor(sf::Color(80, 80, 140, 120));

        const auto& skill = skills[i];
        std::wstring label = L"Skill" + std::to_wstring(i + 1) + L" PP:" + std::to_wstring(skill.pp) + L"/" + std::to_wstring(skill.max_pp);
        sf::Color txt_color = skill.pp > 0 ? sf::Color(200, 200, 230) : sf::Color(100, 100, 120);

        btn.text.setFont(font_);
        btn.text.setString(label);
        btn.text.setCharacterSize(14);
        btn.text.setFillColor(txt_color);
        auto lb = btn.text.getLocalBounds();
        btn.text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        btn.text.setPosition(bx + 100.f, by + 20.f);

        skill_buttons_.push_back(btn);
    }

    back_button_.shape.setSize({100.f, 35.f});
    back_button_.shape.setPosition(430.f, 500.f);
    back_button_.shape.setFillColor(sf::Color(50, 50, 80));
    back_button_.text.setFont(font_);
    back_button_.text.setString(L"Back");
    back_button_.text.setCharacterSize(14);
    back_button_.text.setFillColor(sf::Color(160, 160, 200));
    auto lb = back_button_.text.getLocalBounds();
    back_button_.text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
    back_button_.text.setPosition(480.f, 517.5f);
    back_button_.hover = false;
    (void)lb; // suppress unused warning
}

bool BattleState::contains(const Button& btn, const sf::Vector2f& point) const {
    return btn.shape.getGlobalBounds().contains(point);
}

void BattleState::draw_panel(sf::RenderWindow& window, float x, float y, float w, float h, sf::Color fill) {
    sf::RectangleShape panel({w, h});
    panel.setPosition(x, y);
    panel.setFillColor(fill);
    window.draw(panel);
}

void BattleState::draw_label(sf::RenderWindow& window, const sf::String& text, float x, float y,
                              int size, sf::Color color) {
    sf::Text t;
    t.setFont(font_);
    t.setString(text);
    t.setCharacterSize(size);
    t.setFillColor(color);
    t.setPosition(x, y);
    window.draw(t);
}
