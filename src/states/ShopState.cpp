#include "ShopState.h"
#include "../core/Game.h"
#include "../system/ShopSystem.h"
#include <cmath>

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
}

void ShopState::cleanup() {}

void ShopState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f mouse(static_cast<float>(event.mouseMove.x),
                           static_cast<float>(event.mouseMove.y));

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
        sf::Vector2f mouse(static_cast<float>(event.mouseButton.x),
                           static_cast<float>(event.mouseButton.y));

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
                message_ = L"\u8d2d\u4e70\u6210\u529f\uff01" + sf::String(item.name).toWideString();
                refresh_items();
            } else {
                message_ = L"\u91d1\u5e01\u4e0d\u8db3\uff01";
            }
        }
    }
}

void ShopState::update(float dt) {}

void ShopState::render(sf::RenderWindow& window) {
    window.clear(sf::Color(15, 15, 35));

    // Background bands
    for (int i = 0; i < 8; ++i) {
        sf::RectangleShape band({960.f, 80.f});
        band.setPosition(0.f, i * 80.f);
        sf::Uint8 r = static_cast<sf::Uint8>(15 + i * 2);
        band.setFillColor(sf::Color(r, r, 35 + i * 4));
        window.draw(band);
    }

    // Title
    sf::Text title;
    title.setFont(font_);
    title.setString(L"SHOP");
    title.setCharacterSize(32);
    title.setFillColor(sf::Color(255, 220, 100));
    auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition(480.f, 10.f);
    window.draw(title);

    // Player gold display
    sf::Text gold;
    gold.setFont(font_);
    gold.setString(L"\u91d1\u5e01: " + std::to_wstring(player_.gold()) + L" G");
    gold.setCharacterSize(18);
    gold.setFillColor(sf::Color(255, 220, 100));
    gold.setPosition(700.f, 15.f);
    window.draw(gold);

    // Tab buttons
    for (int i = 0; i < 2; ++i) {
        sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + tab_buttons_[i].hover * 135);
        tab_buttons_[i].shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));
        if (i == static_cast<int>(current_tab_)) {
            tab_buttons_[i].shape.setFillColor(sf::Color(60, 60, 100));
        }
        window.draw(tab_buttons_[i].shape);
        window.draw(tab_buttons_[i].text);
    }

    // Item list
    float start_y = 110.f;
    for (size_t i = 0; i < item_buttons_.size() && i < 8; ++i) {
        draw_item_button(window, {100.f, start_y + i * 55.f}, static_cast<int>(i));
    }

    // Selected item panel
    if (selected_index_ >= 0 && selected_index_ < static_cast<int>(shop_items_.size())) {
        const Item& item = shop_items_[selected_index_];
        float px = 550.f, py = 100.f;

        sf::RectangleShape panel({350.f, 150.f});
        panel.setPosition(px, py);
        panel.setFillColor(sf::Color(25, 25, 50, 230));
        panel.setOutlineThickness(1.5f);
        panel.setOutlineColor(sf::Color(60, 60, 120, 150));
        window.draw(panel);

        sf::Text name;
        name.setFont(font_);
        name.setString(sf::String(item.name).toWideString());
        name.setCharacterSize(20);
        name.setFillColor(sf::Color(255, 220, 100));
        name.setPosition(px + 20.f, py + 15.f);
        window.draw(name);

        sf::Text price;
        price.setFont(font_);
        price.setString(L"\u4ef7\u683c: " + std::to_wstring(item.price) + L" G");
        price.setCharacterSize(16);
        price.setFillColor(sf::Color(200, 200, 220));
        price.setPosition(px + 20.f, py + 50.f);
        window.draw(price);

        sf::Text desc;
        desc.setFont(font_);
        if (item.type == ItemType::Heal50 || item.type == ItemType::Heal100) {
            int heal = (item.type == ItemType::Heal50) ? 50 : 100;
            desc.setString(L"\u6cbb\u7597\u6548\u679c: +\u7f6e" + std::to_wstring(heal) + L" HP");
        } else if (item.type == ItemType::Pokeball) {
            desc.setString(L"\u8bdb\u6355\u91ce\u751f\u7cbe\u7075");
        } else if (item.type == ItemType::SuperBall) {
            desc.setString(L"\u8bdb\u6355\u7387\u66f4\u9ad8");
        } else if (item.type == ItemType::Revive) {
            desc.setString(L"\u590d\u6d3b\u6b7b\u4ea1\u7cbe\u7075");
        } else {
            desc.setString(L"\u8ba9\u7cbe\u7075\u8fdb\u5316");
        }
        desc.setCharacterSize(14);
        desc.setFillColor(sf::Color(140, 140, 180));
        desc.setPosition(px + 20.f, py + 80.f);
        window.draw(desc);

        sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + buy_button_.hover * 135);
        buy_button_.shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));
        window.draw(buy_button_.shape);
        window.draw(buy_button_.text);
    }

    // Back button
    window.draw(back_button_.shape);
    window.draw(back_button_.text);

    // Message bar
    sf::RectangleShape msg_bg({600.f, 35.f});
    msg_bg.setPosition(180.f, 560.f);
    msg_bg.setFillColor(sf::Color(20, 20, 45, 230));
    window.draw(msg_bg);

    sf::Text msg;
    msg.setFont(font_);
    msg.setString(message_);
    msg.setCharacterSize(15);
    msg.setFillColor(sf::Color(255, 230, 120));
    msg.setPosition(195.f, 568.f);
    window.draw(msg);
}

void ShopState::draw_item_button(sf::RenderWindow& window, const sf::Vector2f& pos, int index) {
    if (index >= static_cast<int>(shop_items_.size())) return;

    const Item& item = shop_items_[index];
    Button& btn = item_buttons_[index];

    btn.shape.setSize({420.f, 50.f});
    btn.shape.setPosition(pos);
    btn.shape.setFillColor(selected_index_ == index ? sf::Color(50, 50, 80) : sf::Color(35, 35, 65));
    btn.shape.setOutlineThickness(1.5f);
    sf::Uint8 outline_a = static_cast<sf::Uint8>(120 + btn.hover * 135);
    btn.shape.setOutlineColor(sf::Color(100, 120, 200, outline_a));

    btn.text.setFont(font_);
    std::wstring label = sf::String(item.name).toWideString() + L"  " + std::to_wstring(item.price) + L" G";
    btn.text.setString(label);
    btn.text.setCharacterSize(16);
    btn.text.setFillColor(btn.hover ? sf::Color(255, 255, 255) : sf::Color(200, 200, 230));
    btn.text.setPosition(pos.x + 20.f, pos.y + 15.f);

    window.draw(btn.shape);
    window.draw(btn.text);
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
