#pragma once

#include <SFML/Graphics.hpp>
#include <string>

namespace RetroUI {

// 初始化（加载像素字体）；应在游戏启动后尽早调用一次
bool init(const std::string& font_path = "assets/fonts/zpix.ttf");

sf::Font& get_font();

// 经典宝可梦风格对话框（白底黑边）
void draw_box(sf::RenderWindow& w, float x, float y, float bw, float bh,
              sf::Color fill    = sf::Color::White,
              sf::Color border  = sf::Color::Black,
              float border_thick = 3.f);

// 绘制像素文字（可选右下阴影）
void draw_text(sf::RenderWindow& w, const std::wstring& str,
               float x, float y,
               unsigned int size  = 20,
               sf::Color color    = sf::Color::Black,
               bool shadow        = false);

// 绘制菜单选项（selected == true 时在左侧显示 ► 箭头）
void draw_menu_item(sf::RenderWindow& w, const std::wstring& str,
                    float x, float y,
                    bool selected,
                    unsigned int size = 20);

// 绘制HP条（绿/黄/红渐变）
void draw_hp_bar(sf::RenderWindow& w,
                 float x, float y, float bw, float bh,
                 int hp, int max_hp);

} // namespace RetroUI
