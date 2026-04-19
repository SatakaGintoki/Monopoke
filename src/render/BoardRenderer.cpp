#include "BoardRenderer.h"
#include "../ui/RetroUI.h"
#include <cmath>

BoardRenderer::BoardRenderer() {}

void BoardRenderer::init(const Board& board) {
    calc_positions();
    load_textures();
}

void BoardRenderer::load_textures() {
    // 尝试加载贴图（静默失败，没找到就用默认画法）
    tile_textures_[TileType::Start].loadFromFile("assets/textures/tile_start.png");
    tile_textures_[TileType::Empty].loadFromFile("assets/textures/tile_empty.png");
    tile_textures_[TileType::Wild].loadFromFile("assets/textures/tile_wild.png");
    tile_textures_[TileType::Shop].loadFromFile("assets/textures/tile_shop.png");
    tile_textures_[TileType::Event].loadFromFile("assets/textures/tile_event.png");
    tile_textures_[TileType::Camp].loadFromFile("assets/textures/tile_camp.png");
    tile_textures_[TileType::Battle].loadFromFile("assets/textures/tile_battle.png");

    player1_texture_.loadFromFile("assets/textures/player1.png");
    player2_texture_.loadFromFile("assets/textures/player2.png");
}

void BoardRenderer::calc_positions() {
    positions_.clear();
    positions_.resize(kBoardSize);

    int cols = 6;
    int rows = 4;
    float stride_x = kTileW + kGap;
    float stride_y = kTileH + kGap;

    // 底边（0~5）：从左到右
    for (int i = 0; i < cols; ++i) {
        positions_[i] = {kBoardX + i * stride_x,
                         kBoardY + (rows + 1) * stride_y};
    }
    // 右边（6~9）：从下到上
    for (int i = 0; i < rows; ++i) {
        positions_[cols + i] = {kBoardX + (cols - 1) * stride_x,
                                kBoardY + (rows - i) * stride_y};
    }
    // 顶边（10~15）：从右到左
    for (int i = 0; i < cols; ++i) {
        positions_[cols + rows + i] = {kBoardX + (cols - 1 - i) * stride_x,
                                       kBoardY};
    }
    // 左边（16~19）：从上到下
    for (int i = 0; i < rows; ++i) {
        positions_[cols + rows + cols + i] = {kBoardX,
                                              kBoardY + (i + 1) * stride_y};
    }
}

sf::Vector2f BoardRenderer::tile_center(int index) const {
    if (index < 0 || index >= static_cast<int>(positions_.size())) {
        return {0.f, 0.f};
    }
    return {positions_[index].x + kTileW / 2.f,
            positions_[index].y + kTileH / 2.f};
}

sf::Color BoardRenderer::tile_color(TileType type) const {
    switch (type) {
        case TileType::Start:   return sf::Color(220, 180, 50);
        case TileType::Empty:   return sf::Color(55, 95, 65);
        case TileType::Wild:    return sf::Color(150, 45, 45);
        case TileType::Shop:    return sf::Color(45, 90, 150);
        case TileType::Event:   return sf::Color(130, 60, 130);
        case TileType::Camp:    return sf::Color(45, 130, 100);
        case TileType::Battle:  return sf::Color(170, 85, 30);
        default:                return sf::Color(70, 70, 70);
    }
}

sf::Color BoardRenderer::tile_color_dark(TileType type) const {
    sf::Color c = tile_color(type);
    return sf::Color(c.r / 2, c.g / 2, c.b / 2);
}

std::wstring BoardRenderer::tile_label(TileType type) const {
    switch (type) {
        case TileType::Start:   return L"\u8d77\u70b9";
        case TileType::Empty:   return L"\u7a7a\u5730";
        case TileType::Wild:    return L"\u91ce\u602a";
        case TileType::Shop:    return L"\u5546\u5e97";
        case TileType::Event:   return L"\u4e8b\u4ef6";
        case TileType::Camp:    return L"\u8425\u5730";
        case TileType::Battle:  return L"\u5bf9\u6218";
        default:                return L"?";
    }
}

std::wstring BoardRenderer::tile_icon(TileType type) const {
    switch (type) {
        case TileType::Start:   return L"\u2605";   // ★
        case TileType::Wild:    return L"\u2620";   // ☠
        case TileType::Shop:    return L"\u2668";   // ♨
        case TileType::Event:   return L"?";
        case TileType::Camp:    return L"\u2665";   // ♥
        case TileType::Battle:  return L"\u2694";   // ⚔
        default:                return L"";
    }
}

void BoardRenderer::draw_board_frame(sf::RenderWindow& window) {
    // 棋盘中央区域背景
    float left = kBoardX + kTileW + kGap;
    float top = kBoardY + kTileH + kGap;
    float right = kBoardX + 4 * (kTileW + kGap);
    float bottom = kBoardY + 4 * (kTileH + kGap);

    RetroUI::draw_box(window, left, top, right - left, bottom - top,
                      sf::Color::White, sf::Color(40, 40, 40), 4.f);

    // 中央标题
    RetroUI::draw_text(window, L"地产兽域",
                       (left + right) / 2.f - 60.f, (top + bottom) / 2.f - 20.f,
                       32, sf::Color(40, 40, 40), true);

    RetroUI::draw_text(window, L"MONOPOKE",
                       (left + right) / 2.f - 40.f, (top + bottom) / 2.f + 20.f,
                       16, sf::Color(100, 100, 100));
}

void BoardRenderer::draw_tile(sf::RenderWindow& window, const Tile& tile, int index) {
    sf::Vector2f pos = positions_[index];

    // 如果有贴图，优先画贴图
    if (tile_textures_.count(tile.type()) && tile_textures_[tile.type()].getSize().x > 0) {
        sf::Sprite sprite(tile_textures_[tile.type()]);
        // 自动缩放到格子大小
        sprite.setScale(kTileW / sprite.getTexture()->getSize().x,
                        kTileH / sprite.getTexture()->getSize().y);
        sprite.setPosition(pos);
        window.draw(sprite);
    } else {
        // 没有贴图，回退到经典宝可梦白底黑边格子
        RetroUI::draw_box(window, pos.x, pos.y, kTileW, kTileH,
                          sf::Color::White, sf::Color(40, 40, 40), 2.f);

        // 顶部色条（类型标识）
        sf::RectangleShape bar({kTileW - 4.f, 4.f});
        bar.setPosition(pos.x + 2.f, pos.y + 2.f);
        sf::Color bc = tile_color(tile.type());
        bar.setFillColor(bc);
        window.draw(bar);

        // 格子名称
        RetroUI::draw_text(window, tile_label(tile.type()),
                           pos.x + kTileW / 2.f - 16.f, pos.y + 10.f,
                           14, sf::Color(40, 40, 40));

        // 空地显示价格
        if (tile.type() == TileType::Empty && !tile.has_owner()) {
            RetroUI::draw_text(window, std::to_wstring(tile.price()) + L"G",
                               pos.x + kTileW / 2.f - 14.f, pos.y + 34.f,
                               12, sf::Color(100, 100, 100));
        }
    }

    // 有主人时，无论有没有贴图，都画一层占领框
    if (tile.has_owner()) {
        sf::Color oc = (tile.owner_id() == kPlayer1Id)
            ? sf::Color(80, 180, 255) : sf::Color(255, 140, 80);
        // 玩家拥有的地皮内边框染色
        sf::RectangleShape inner({kTileW - 8.f, kTileH - 8.f});
        inner.setPosition(pos.x + 4.f, pos.y + 4.f);
        inner.setFillColor(sf::Color::Transparent);
        inner.setOutlineColor(oc);
        inner.setOutlineThickness(2.f);
        window.draw(inner);

        int lv = static_cast<int>(tile.level());
        // 等级小方块
        for (int j = 0; j < lv; ++j) {
            sf::RectangleShape dot({8.f, 4.f});
            dot.setPosition(pos.x + 14.f + j * 14.f, pos.y + kTileH - 10.f);
            dot.setFillColor(oc);
            window.draw(dot);
        }
    }

    // 格子编号（小字）
    RetroUI::draw_text(window, std::to_wstring(index),
                       pos.x + 4.f, pos.y + kTileH - 16.f,
                       10, sf::Color(160, 160, 160));
}

void BoardRenderer::draw_player_token(sf::RenderWindow& window, const Player& p,
                                       sf::Color color, float offset_x) {
    sf::Vector2f center = tile_center(p.position());
    sf::Texture* tex = (p.id() == 0) ? &player1_texture_ : &player2_texture_;

    if (tex->getSize().x > 0) {
        // 渲染玩家贴图
        sf::Sprite sprite(*tex);
        // 假设贴图缩放到 48x48
        sprite.setScale(48.f / tex->getSize().x, 48.f / tex->getSize().y);
        sprite.setPosition(center.x - 24.f + offset_x, center.y - 24.f);
        window.draw(sprite);

        // 玩家编号（浮在头顶）
        RetroUI::draw_text(window, L"P" + std::to_wstring(p.id() + 1),
                           center.x + offset_x - 8.f, center.y - 40.f,
                           12, sf::Color(40, 40, 40));
    } else {
        // 阴影
        sf::CircleShape shadow(11.f);
        shadow.setFillColor(sf::Color(0, 0, 0, 80));
        shadow.setPosition(center.x - 11.f + offset_x + 1.f, center.y - 11.f + 2.f);
        window.draw(shadow);

        // 主体
        sf::CircleShape circle(11.f);
        circle.setFillColor(color);
        circle.setOutlineThickness(2.f);
        circle.setOutlineColor(sf::Color(40, 40, 40));
        circle.setPosition(center.x - 11.f + offset_x, center.y - 11.f);
        window.draw(circle);

        // 玩家编号
        RetroUI::draw_text(window, std::to_wstring(p.id() + 1),
                           center.x + offset_x - 4.f, center.y - 8.f,
                           12, sf::Color::White);
    }
}

void BoardRenderer::render(sf::RenderWindow& window, const Board& board,
                           const Player& p1, const Player& p2) {
    // 棋盘中央
    draw_board_frame(window);

    // 格子
    for (int i = 0; i < kBoardSize; ++i) {
        draw_tile(window, board.tile_at(i), i);
    }

    // 玩家棋子
    draw_player_token(window, p1, sf::Color(60, 140, 230), -14.f);
    draw_player_token(window, p2, sf::Color(230, 110, 60),  14.f);
}
