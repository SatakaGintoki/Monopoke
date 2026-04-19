#include "MenuState.h"
#include "PlayState.h"
#include "UITestState.h"
#include "../core/Game.h"
#include "../utils/SaveManager.h"
#include "../utils/AudioManager.h"
#include "../ui/RetroUI.h"
#include <iostream>

// ── 布局常量 ────────────────────────────────────────────────────────────
static constexpr float kWinW = 960.f;
static constexpr float kWinH = 640.f;

// 菜单框 (右下角偏上)
static constexpr float kMenuX   = 600.f;
static constexpr float kMenuY   = 240.f;
static constexpr float kMenuW   = 320.f;
static constexpr float kMenuH   = 270.f;
static constexpr float kItemPadX= 40.f;
static constexpr float kItemPadY= 24.f;
static constexpr float kItemH   = 44.f;

// 说明框 (底部通栏)
static constexpr float kTipX    = 20.f;
static constexpr float kTipY    = 480.f;
static constexpr float kTipW    = 920.f;
static constexpr float kTipH    = 140.f;

// 菜单文字
static const wchar_t* kLabels[5] = {
    L"单机 vs AI",
    L"本地双人",
    L"继续游戏",
    L"UI测试",
    L"退出"
};

// 对应说明文字
static const wchar_t* kTips[5] = {
    L"与电脑 AI 对战的单人游戏模式",
    L"两名玩家轮流使用同一键盘",
    L"读取上次存档继续游戏",
    L"快速进入商店/战斗/结算界面",
    L"退出游戏"
};

// ── 经典宝可梦配色 ────────────────────────────────────────────────────────
static const sf::Color kBgColor     {248, 248, 240};  // 极浅的米白
static const sf::Color kBoxFill     {255, 255, 255};  // 纯白底
static const sf::Color kBoxBorder   {40,  40,  40};   // 深灰/黑边
static const sf::Color kTextColor   {40,  40,  40};   // 纯黑字

// ────────────────────────────────────────────────────────────────────────
void MenuState::init() {
    RetroUI::init("assets/fonts/zpix.ttf");
    AudioManager::get().play_music(MusicType::Menu);

    // 构建菜单项点击区域
    item_rects_.clear();
    for (int i = 0; i < 5; ++i) {
        sf::FloatRect r;
        r.left   = kMenuX + kItemPadX;
        r.top    = kMenuY + kItemPadY + i * kItemH;
        r.width  = kMenuW - kItemPadX * 2.f;
        r.height = kItemH;
        item_rects_.push_back(r);
    }
}

// ────────────────────────────────────────────────────────────────────────
void MenuState::handle_event(const sf::Event& event) {
    // 键盘导航
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Up) {
            selected_ = (selected_ + 4) % 5;
        } else if (event.key.code == sf::Keyboard::Down) {
            selected_ = (selected_ + 1) % 5;
        } else if (event.key.code == sf::Keyboard::Return ||
                   event.key.code == sf::Keyboard::Space) {
            goto confirm;
        }
    }

    // 鼠标移动 → 更新高亮
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mp = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        for (int i = 0; i < 5; ++i) {
            if (item_rects_[i].contains(mp)) {
                selected_ = i;
                break;
            }
        }
    }

    // 鼠标点击
    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mp = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
        for (int i = 0; i < 5; ++i) {
            if (item_rects_[i].contains(mp)) {
                selected_ = i;
                goto confirm;
            }
        }
    }

    return;

confirm:
    switch (selected_) {
        case 0:
            switch_state(Game::get().state_manager(),
                         new PlayState(false, GameMode::PvE));
            return;
        case 1:
            switch_state(Game::get().state_manager(),
                         new PlayState(false, GameMode::PvP));
            return;
        case 2:
            if (SaveManager::get().has_save(SaveManager::kAutosavePath)) {
                switch_state(Game::get().state_manager(), new PlayState(true));
                return;
            } else {
                std::cerr << "[MenuState] 没有存档: "
                          << SaveManager::kAutosavePath << "\n";
            }
            break;
        case 3:
            switch_state(Game::get().state_manager(), new UITestState());
            return;
        case 4:
            Game::get().exit();
            return;
        default:
            break;
    }
}

// ────────────────────────────────────────────────────────────────────────
void MenuState::update(float dt) {
    time_ += dt;
}

// ────────────────────────────────────────────────────────────────────────
void MenuState::render(sf::RenderWindow& window) {
    // ── 纯色平铺背景 ──────────────────────────────────────────
    sf::RectangleShape bg({kWinW, kWinH});
    bg.setFillColor(kBgColor);
    window.draw(bg);

    // ── 标题 ──────────────────────────────────────────────────
    // 居中大字，无框，带经典阴影
    RetroUI::draw_text(window, L"地产兽域",
                       200.f, 100.f,
                       64, kTextColor, true);

    RetroUI::draw_text(window, L"MONOPOKE  v0.2.0",
                       204.f, 180.f,
                       20, sf::Color(100, 100, 100));

    // ── 菜单框 ────────────────────────────────────────────────
    RetroUI::draw_box(window,
                      kMenuX, kMenuY,
                      kMenuW, kMenuH,
                      kBoxFill, kBoxBorder, 4.f);

    for (int i = 0; i < 5; ++i) {
        float ix = kMenuX + kItemPadX;
        float iy = kMenuY + kItemPadY + i * kItemH;
        RetroUI::draw_menu_item(window, kLabels[i], ix, iy,
                                i == selected_, 24);
    }

    // ── 说明框 (底部通栏对话框) ───────────────────────────────
    RetroUI::draw_box(window,
                      kTipX, kTipY,
                      kTipW, kTipH,
                      kBoxFill, kBoxBorder, 4.f);

    RetroUI::draw_text(window, kTips[selected_],
                       kTipX + 36.f, kTipY + 36.f,
                       24, kTextColor);

    // ── 操作提示（右下角） ────────────────────────────────────
    RetroUI::draw_text(window,
                       L"↑↓ 选择    Enter 确认",
                       kWinW - 280.f, kWinH - 40.f,
                       16, sf::Color(120, 120, 120));
}
