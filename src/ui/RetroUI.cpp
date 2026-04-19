#include "RetroUI.h"
#include <iostream>

namespace RetroUI {

static sf::Font g_font;
static bool g_initialized = false;

bool init(const std::string& font_path) {
    if (g_initialized) return true;

    if (!g_font.loadFromFile(font_path)) {
        // 回退到系统中文字体
        if (!g_font.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
            g_font.loadFromFile("C:/Windows/Fonts/arial.ttf");
        }
        std::cerr << "[RetroUI] 像素字体加载失败: " << font_path
                  << "，已回退到系统字体\n";
    }

    g_initialized = true;
    return true;
}

sf::Font& get_font() {
    return g_font;
}

void draw_box(sf::RenderWindow& w, float x, float y, float bw, float bh,
              sf::Color fill, sf::Color border, float border_thick) {
    // 外层主边框（向内）
    sf::RectangleShape rect({bw, bh});
    rect.setPosition(x, y);
    rect.setFillColor(fill);
    rect.setOutlineColor(border);
    rect.setOutlineThickness(-border_thick);
    w.draw(rect);

    // 经典的宝可梦双层内边框（细线）
    float inner_offset = border_thick + 2.f;
    sf::RectangleShape inner({bw - inner_offset * 2, bh - inner_offset * 2});
    inner.setPosition(x + inner_offset, y + inner_offset);
    inner.setFillColor(sf::Color::Transparent);
    inner.setOutlineColor(border);
    inner.setOutlineThickness(-2.f);
    w.draw(inner);
}

void draw_text(sf::RenderWindow& w, const std::wstring& str,
               float x, float y, unsigned int size,
               sf::Color color, bool shadow) {
    sf::Text t;
    t.setFont(g_font);
    t.setString(str);
    t.setCharacterSize(size);

    if (shadow) {
        t.setFillColor(sf::Color(60, 60, 60, 160));
        t.setPosition(x + 2.f, y + 2.f);
        w.draw(t);
    }

    t.setFillColor(color);
    t.setPosition(x, y);
    w.draw(t);
}

void draw_menu_item(sf::RenderWindow& w, const std::wstring& str,
                    float x, float y, bool selected, unsigned int size) {
    if (selected) {
        draw_text(w, L"► " + str, x, y, size, sf::Color::Black);
    } else {
        // 用全角空格对齐（避免箭头宽度影响布局）
        draw_text(w, L"  " + str, x, y, size, sf::Color(60, 60, 60));
    }
}

void draw_hp_bar(sf::RenderWindow& w,
                 float x, float y, float bw, float bh,
                 int hp, int max_hp) {
    // 背景
    sf::RectangleShape bg({bw, bh});
    bg.setPosition(x, y);
    bg.setFillColor(sf::Color(80, 80, 80));
    w.draw(bg);

    float ratio = (max_hp > 0)
                      ? static_cast<float>(hp) / static_cast<float>(max_hp)
                      : 0.f;
    if (ratio < 0.f) ratio = 0.f;
    if (ratio > 1.f) ratio = 1.f;

    sf::Color hp_color = (ratio > 0.5f)  ? sf::Color(72, 200, 72)
                       : (ratio > 0.25f) ? sf::Color(240, 200, 40)
                                         : sf::Color(240, 60,  60);

    sf::RectangleShape fill({bw * ratio, bh});
    fill.setPosition(x, y);
    fill.setFillColor(hp_color);
    w.draw(fill);
}

} // namespace RetroUI
