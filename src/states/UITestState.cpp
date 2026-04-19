#include "UITestState.h"
#include "MenuState.h"
#include "ShopState.h"
#include "BattleState.h"
#include "ResultState.h"
#include "../core/Game.h"
#include "../system/MonsterSystem.h"
#include "../ui/RetroUI.h"

namespace {
constexpr float kWinW = 960.f;
constexpr float kWinH = 640.f;

constexpr float kMenuX = 220.f;
constexpr float kMenuY = 170.f;
constexpr float kMenuW = 520.f;
constexpr float kMenuH = 280.f;
constexpr float kItemPadX = 26.f;
constexpr float kItemPadY = 24.f;
constexpr float kItemH = 48.f;

constexpr int kItemCount = 5;
const wchar_t* kItems[kItemCount] = {
    L"商店界面测试",
    L"战斗界面测试（野怪）",
    L"结算界面测试（玩家1胜）",
    L"结算界面测试（玩家2胜）",
    L"返回主菜单"
};
}

void UITestState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }
    RetroUI::init("assets/fonts/zpix.ttf");

    item_rects_.clear();
    for (int i = 0; i < kItemCount; ++i) {
        item_rects_.push_back(sf::FloatRect(
            kMenuX + kItemPadX,
            kMenuY + kItemPadY + i * kItemH,
            kMenuW - kItemPadX * 2.f,
            kItemH));
    }

    test_player_.set_gold(5000);
    player_mon_ = MonsterSystem::get().create_monster(1, 12);
    enemy_mon_ = MonsterSystem::get().create_monster(2, 11);
}

void UITestState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up) {
            selected_ = (selected_ + kItemCount - 1) % kItemCount;
        } else if (event.key.code == sf::Keyboard::Down) {
            selected_ = (selected_ + 1) % kItemCount;
        } else if (event.key.code == sf::Keyboard::Return || event.key.code == sf::Keyboard::Space) {
            enter_selected();
            return;
        } else if (event.key.code == sf::Keyboard::Escape) {
            switch_state(Game::get().state_manager(), new MenuState());
            return;
        }
    }

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mp = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (int i = 0; i < kItemCount; ++i) {
            if (item_rects_[i].contains(mp)) {
                selected_ = i;
                break;
            }
        }
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mp = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        for (int i = 0; i < kItemCount; ++i) {
            if (item_rects_[i].contains(mp)) {
                selected_ = i;
                enter_selected();
                return;
            }
        }
    }
}

void UITestState::enter_selected() {
    switch (selected_) {
        case 0:
            push_state(Game::get().state_manager(), new ShopState(test_player_));
            return;
        case 1:
            push_state(Game::get().state_manager(),
                       new BattleState(player_mon_, enemy_mon_, BattleType::Wild, false));
            return;
        case 2:
            push_state(Game::get().state_manager(),
                       new ResultState(GameResult::Player1Win, 0));
            return;
        case 3:
            push_state(Game::get().state_manager(),
                       new ResultState(GameResult::Player2Win, 1));
            return;
        case 4:
            switch_state(Game::get().state_manager(), new MenuState());
            return;
        default:
            return;
    }
}

void UITestState::update(float dt) {
    (void)dt;
}

void UITestState::render(sf::RenderWindow& window) {
    sf::RectangleShape bg({kWinW, kWinH});
    bg.setFillColor(sf::Color(248, 248, 240));
    window.draw(bg);

    RetroUI::draw_text(window, L"UI 测试通道", 290.f, 80.f, 56, sf::Color(40, 40, 40), true);

    RetroUI::draw_box(window, kMenuX, kMenuY, kMenuW, kMenuH,
                      sf::Color::White, sf::Color(40, 40, 40), 4.f);
    for (int i = 0; i < kItemCount; ++i) {
        RetroUI::draw_menu_item(window, kItems[i],
                                kMenuX + kItemPadX,
                                kMenuY + kItemPadY + i * kItemH,
                                i == selected_, 24);
    }

    RetroUI::draw_box(window, 20.f, 500.f, 920.f, 120.f,
                      sf::Color::White, sf::Color(40, 40, 40), 4.f);
    RetroUI::draw_text(window, L"用于快速调试界面，避免完整开局流程。", 45.f, 540.f, 22, sf::Color(40, 40, 40));
}
