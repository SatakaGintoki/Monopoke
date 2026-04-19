#include "ShopState.h"
#include "../core/Game.h"
#include "../system/ShopSystem.h"
#include "../ui/RetroUI.h"
#include <cmath>

namespace {
std::wstring utf8_to_wstring(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end()).toWideString();
}
}

ShopState::ShopState(Player& player) : player_(player) {}

void ShopState::init() {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }

    ShopSystem::get().load_from_json("assets/data/items.json");
    refresh_items();

    // Tab buttons
    const wchar_t* tab_labels[] = { L"\u6d88\u8017\u54c1", L"\u8fdb\u5316\u9053\u5177" };
    for (int i = 0; i < 2; ++i) {
        tab_buttons_[i].shape.setSize({180.f, 40.f});
        tab_buttons_[i].shape.setPosition(100.f + i * 190.f, 50.f);
        tab_buttons_[i].shape.setFillColor(sf::Color(35, 35, 65));
        tab_buttons_[i].shape.setOutlineThickness(1.5f);
        tab_buttons_[i].shape.setOutlineColor(sf::Color(80, 80, 140, 120));

        tab_buttons_[i].text.setFont(font_);
        tab_buttons_[i].text.setString(tab_labels[i]);
        tab_buttons_[i].text.setCharacterSize(16);
        tab_buttons_[i].text.setFillColor(sf::Color(200, 200, 230));
        auto lb = tab_buttons_[i].text.getLocalBounds();
        tab_buttons_[i].text.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
        tab_buttons_[i].text.setPosition(100.f + i * 190.f + 90.f, 60.f);
        tab_buttons_[i].hover = false;
    }

    // Back button
    back_button_.shape.setSize({120.f, 40.f});
    back_button_.shape.setPosition(750.f, 560.f);
    back_button_.shape.setFillColor(sf::Color(50, 50, 80));
    back_button_.text.setFont(font_);
    back_button_.text.setString(L"\u8fd4\u56de");
    back_button_.text.setCharacterSize(16);
    back_button_.text.setFillColor(sf::Color(160, 160, 200));
    auto blb = back_button_.text.getLocalBounds();
    back_button_.text.setOrigin(blb.left + blb.width / 2.f, blb.top + blb.height / 2.f);
    back_button_.text.setPosition(750.f + 60.f, 570.f);
    back_button_.hover = false;

    // Buy button
    buy_button_.shape.setSize({150.f, 45.f});
    buy_button_.shape.setPosition(550.f, 500.f);
    buy_button_.shape.setFillColor(sf::Color(35, 35, 65));
    buy_button_.shape.setOutlineThickness(1.5f);
    buy_button_.shape.setOutlineColor(sf::Color(80, 80, 140, 120));
    buy_button_.text.setFont(font_);
    buy_button_.text.setString(L"\u8d2d\u4e70");
    buy_button_.text.setCharacterSize(18);
    buy_button_.text.setFillColor(sf::Color(200, 200, 230));
    auto blb2 = buy_button_.text.getLocalBounds();
    buy_button_.text.setOrigin(blb2.left + blb2.width / 2.f, blb2.top + blb2.height / 2.f);
    buy_button_.text.setPosition(550.f + 75.f, 512.f);
    buy_button_.hover = false;

    message_ = L"\u6b22\u8fce\u5149\u4e34\u5546\u5e97\uff01";
    ui_view_.setSize(1440.f, 960.f);
    ui_view_.setCenter(720.f, 480.f);
}

void ShopState::cleanup() {}

void ShopState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseMove.x, event.mouseMove.y), ui_view_);

        for (auto& btn : item_buttons_) {
            btn.hover = contains(btn, mouse);
        }
        for (int i = 0; i < 2; ++i) {
            tab_buttons_[i].hover = tab_buttons_[i].shape.getGlobalBounds().contains(mouse);
        }
        back_button_.hover = back_button_.shape.getGlobalBounds().contains(mouse);
        buy_button_.hover = buy_button_.shape.getGlobalBounds().contains(mouse);
    }

    if (event.type == sf::Event::MouseButtonPressed &&
        event.mouseButton.button == sf::Mouse::Left) {
        sf::Vector2f mouse = Game::get().window().mapPixelToCoords(
            sf::Vector2i(event.mouseButton.x, event.mouseButton.y), ui_view_);

        // Tab switching
        for (int i = 0; i < 2; ++i) {
            if (tab_buttons_[i].hover) {
                current_tab_ = (i == 0) ? Tab::Consumable : Tab::Evolution;
                selected_index_ = -1;
                refresh_items();
                return;
            }
        }

        // Back button
        if (back_button_.hover) {
            pop_state(Game::get().state_manager());
            return;
        }

        // Item selection
        for (size_t i = 0; i < item_buttons_.size(); ++i) {
            if (contains(item_buttons_[i], mouse)) {
                selected_index_ = static_cast<int>(i);
                return;
            }
        }

        // Buy button
        if (buy_button_.hover && selected_index_ >= 0 && selected_index_ < static_cast<int>(shop_items_.size())) {
            const Item& item = shop_items_[selected_index_];
            if (ShopSystem::get().buy_item(player_, item, 1)) {
                message_ = L"\u8d2d\u4e70\u6210\u529f\uff01" + utf8_to_wstring(item.name);
                refresh_items();
            } else {
                message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff01";
            }
        }
    }
}

void ShopState::update(float dt) {}

void ShopState::render(sf::RenderWindow& window) {
    window.setView(ui_view_);
    // 纯色背景
    sf::RectangleShape bg({1440.f, 960.f});
    bg.setFillColor(sf::Color(248, 248, 240));
    window.draw(bg);

    // 标题
    RetroUI::draw_text(window, L"商店", 480.f - 40.f, 20.f, 32, sf::Color(40, 40, 40), true);

    // 玩家金币
    std::wstring gold_str = L"拥有金币: " + std::to_wstring(player_.gold()) + L" G";
    RetroUI::draw_text(window, gold_str, 750.f, 30.f, 20, sf::Color(40, 40, 40));

    // Tabs
    for (int i = 0; i < 2; ++i) {
        bool is_active = (i == 0 && current_tab_ == Tab::Consumable) ||
                         (i == 1 && current_tab_ == Tab::Evolution);
        RetroUI::draw_box(window, tab_buttons_[i].shape.getPosition().x, tab_buttons_[i].shape.getPosition().y,
                          tab_buttons_[i].shape.getSize().x, tab_buttons_[i].shape.getSize().y,
                          is_active ? sf::Color(200, 200, 200) : sf::Color::White,
                          sf::Color(40, 40, 40), 2.f);
        RetroUI::draw_text(window, tab_buttons_[i].text.getString().toWideString(),
                           tab_buttons_[i].shape.getPosition().x + 20.f, tab_buttons_[i].shape.getPosition().y + 10.f,
                           18, sf::Color(40, 40, 40));
    }

    // Item list
    RetroUI::draw_box(window, 80.f, 100.f, 440.f, 450.f, sf::Color::White, sf::Color(40, 40, 40), 4.f);
    float start_y = 110.f;
    for (size_t i = 0; i < item_buttons_.size() && i < 8; ++i) {
        draw_item_button(window, {100.f, start_y + i * 55.f}, static_cast<int>(i));
    }

    // Selected item panel
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(shop_items_.size())) {
        const Item& item = shop_items_[selected_index_];
        float px = 550.f, py = 100.f;

        RetroUI::draw_box(window, px, py, 350.f, 150.f, sf::Color::White, sf::Color(40, 40, 40), 4.f);

        RetroUI::draw_text(window, utf8_to_wstring(item.name), px + 20.f, py + 15.f, 24, sf::Color(40, 40, 40));
        RetroUI::draw_text(window, L"价格: " + std::to_wstring(item.price) + L" G", px + 20.f, py + 50.f, 18, sf::Color(100, 100, 100));

        std::wstring desc_str;
        if (item.type == ItemType::Heal50 || item.type == ItemType::Heal100) {
            int heal = (item.type == ItemType::Heal50) ? 50 : 100;
            desc_str = L"治疗效果: +恢复" + std::to_wstring(heal) + L" HP";
        } else if (item.type == ItemType::Pokeball) {
            desc_str = L"捕捉野生精灵";
        } else if (item.type == ItemType::SuperBall) {
            desc_str = L"捕捉率更高";
        } else if (item.type == ItemType::Revive) {
            desc_str = L"复活死亡精灵";
        } else {
            desc_str = L"让精灵进化";
        }
        RetroUI::draw_text(window, desc_str, px + 20.f, py + 80.f, 16, sf::Color(80, 80, 80));

        RetroUI::draw_box(window, buy_button_.shape.getPosition().x, buy_button_.shape.getPosition().y,
                          buy_button_.shape.getSize().x, buy_button_.shape.getSize().y,
                          buy_button_.hover ? sf::Color(220, 220, 220) : sf::Color::White,
                          sf::Color(40, 40, 40), 2.f);
        RetroUI::draw_text(window, buy_button_.text.getString().toWideString(),
                           buy_button_.shape.getPosition().x + 50.f, buy_button_.shape.getPosition().y + 10.f,
                           18, sf::Color(40, 40, 40));
    }

    // Back button
    RetroUI::draw_box(window, back_button_.shape.getPosition().x, back_button_.shape.getPosition().y,
                      back_button_.shape.getSize().x, back_button_.shape.getSize().y,
                      back_button_.hover ? sf::Color(220, 220, 220) : sf::Color::White,
                      sf::Color(40, 40, 40), 2.f);
    RetroUI::draw_text(window, back_button_.text.getString().toWideString(),
                       back_button_.shape.getPosition().x + 40.f, back_button_.shape.getPosition().y + 10.f,
                       18, sf::Color(40, 40, 40));

    // Message bar
    RetroUI::draw_box(window, 180.f, 560.f, 600.f, 50.f, sf::Color::White, sf::Color(40, 40, 40), 4.f);
    RetroUI::draw_text(window, message_, 200.f, 575.f, 18, sf::Color(40, 40, 40));
}

void ShopState::draw_item_button(sf::RenderWindow& window, const sf::Vector2f& pos, int index) {
    if (index >= static_cast<int>(shop_items_.size())) return;

    const Item& item = shop_items_[index];
    Button& btn = item_buttons_[index];

    btn.shape.setSize({400.f, 50.f});
    btn.shape.setPosition(pos);

    RetroUI::draw_box(window, pos.x, pos.y, 400.f, 50.f,
                      selected_index_ == index ? sf::Color(200, 200, 200) : sf::Color::White,
                      sf::Color(40, 40, 40), 2.f);

    std::wstring label = utf8_to_wstring(item.name) + L"  " + std::to_wstring(item.price) + L" G";
    RetroUI::draw_text(window, label, pos.x + 20.f, pos.y + 15.f, 18, sf::Color(40, 40, 40));
}

bool ShopState::contains(const Button& btn, const sf::Vector2f& point) const {
    return btn.shape.getGlobalBounds().contains(point);
}

void ShopState::refresh_items() {
    item_buttons_.clear();
    if (current_tab_ == Tab::Consumable) {
        shop_items_ = ShopSystem::get().get_items_by_category("consumable");
    } else {
        shop_items_ = ShopSystem::get().get_items_by_category("evolution");
    }

    item_buttons_.resize(shop_items_.size());
}
