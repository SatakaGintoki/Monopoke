#include "BoardRenderer.h"
#include <cmath>

BoardRenderer::BoardRenderer() {}

void BoardRenderer::init(const Board& board) {
    if (!font_.loadFromFile("C:/Windows/Fonts/msyh.ttc")) {
        font_.loadFromFile("C:/Windows/Fonts/arial.ttf");
    }
    calc_positions();
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

    sf::RectangleShape center({right - left, bottom - top});
    center.setPosition(left, top);
    center.setFillColor(sf::Color(18, 18, 35, 180));
    center.setOutlineThickness(1.f);
    center.setOutlineColor(sf::Color(40, 40, 80, 100));
    window.draw(center);

    // 中央标题
    sf::Text title;
    title.setFont(font_);
    title.setString(L"\u5730\u4ea7\u517d\u57df");
    title.setCharacterSize(28);
    title.setFillColor(sf::Color(100, 100, 160, 120));
    auto tb = title.getLocalBounds();
    title.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
    title.setPosition((left + right) / 2.f, (top + bottom) / 2.f - 10.f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font_);
    sub.setString("MONOPOKE");
    sub.setCharacterSize(12);
    sub.setFillColor(sf::Color(80, 80, 120, 100));
    auto sb = sub.getLocalBounds();
    sub.setOrigin(sb.left + sb.width / 2.f, sb.top + sb.height / 2.f);
    sub.setPosition((left + right) / 2.f, (top + bottom) / 2.f + 20.f);
    window.draw(sub);
}

void BoardRenderer::draw_tile(sf::RenderWindow& window, const Tile& tile, int index) {
    sf::Vector2f pos = positions_[index];

    // 底色阴影
    sf::RectangleShape shadow({kTileW, kTileH});
    shadow.setPosition(pos.x + 2.f, pos.y + 2.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 60));
    window.draw(shadow);

    // 主体
    sf::RectangleShape rect({kTileW, kTileH});
    rect.setPosition(pos);
    rect.setFillColor(tile_color(tile.type()));
    rect.setOutlineThickness(1.5f);

    if (tile.has_owner()) {
        sf::Color oc = (tile.owner_id() == kPlayer1Id)
            ? sf::Color(80, 180, 255, 200) : sf::Color(255, 140, 80, 200);
        rect.setOutlineColor(oc);
    } else {
        rect.setOutlineColor(sf::Color(255, 255, 255, 30));
    }
    window.draw(rect);

    // 顶部色条（类型标识）
    sf::RectangleShape bar({kTileW, 3.f});
    bar.setPosition(pos);
    sf::Color bc = tile_color(tile.type());
    bar.setFillColor(sf::Color(
        std::min(255, bc.r + 60),
        std::min(255, bc.g + 60),
        std::min(255, bc.b + 60), 200));
    window.draw(bar);

    // 格子名称
    sf::Text label;
    label.setFont(font_);
    label.setString(tile_label(tile.type()));
    label.setCharacterSize(13);
    label.setFillColor(sf::Color(240, 240, 240));
    auto lb = label.getLocalBounds();
    label.setOrigin(lb.left + lb.width / 2.f, 0.f);
    label.setPosition(pos.x + kTileW / 2.f, pos.y + 7.f);
    window.draw(label);

    // 空地显示价格
    if (tile.type() == TileType::Empty && !tile.has_owner()) {
        sf::Text price;
        price.setFont(font_);
        price.setString(std::to_string(tile.price()) + "G");
        price.setCharacterSize(11);
        price.setFillColor(sf::Color(200, 200, 150, 180));
        auto pb = price.getLocalBounds();
        price.setOrigin(pb.left + pb.width / 2.f, 0.f);
        price.setPosition(pos.x + kTileW / 2.f, pos.y + 34.f);
        window.draw(price);
    }

    // 有主人显示等级条
    if (tile.has_owner()) {
        int lv = static_cast<int>(tile.level());
        // 等级小方块
        for (int j = 0; j < lv; ++j) {
            sf::RectangleShape dot({8.f, 4.f});
            dot.setPosition(pos.x + 14.f + j * 14.f, pos.y + kTileH - 10.f);
            sf::Color dc = (tile.owner_id() == kPlayer1Id)
                ? sf::Color(80, 180, 255) : sf::Color(255, 140, 80);
            dot.setFillColor(dc);
            window.draw(dot);
        }
    }

    // 格子编号（小字）
    sf::Text num;
    num.setFont(font_);
    num.setString(std::to_string(index));
    num.setCharacterSize(9);
    num.setFillColor(sf::Color(255, 255, 255, 50));
    num.setPosition(pos.x + 3.f, pos.y + kTileH - 14.f);
    window.draw(num);
}

void BoardRenderer::draw_player_token(sf::RenderWindow& window, const Player& p,
                                       sf::Color color, float offset_x) {
    sf::Vector2f center = tile_center(p.position());

    // 阴影
    sf::CircleShape shadow(11.f);
    shadow.setFillColor(sf::Color(0, 0, 0, 80));
    shadow.setPosition(center.x - 11.f + offset_x + 1.f, center.y - 11.f + 2.f);
    window.draw(shadow);

    // 主体
    sf::CircleShape circle(11.f);
    circle.setFillColor(color);
    circle.setOutlineThickness(2.f);
    circle.setOutlineColor(sf::Color(255, 255, 255, 200));
    circle.setPosition(center.x - 11.f + offset_x, center.y - 11.f);
    window.draw(circle);

    // 玩家编号
    sf::Text label;
    label.setFont(font_);
    label.setString(std::to_string(p.id() + 1));
    label.setCharacterSize(12);
    label.setFillColor(sf::Color::White);
    label.setStyle(sf::Text::Bold);
    auto lb = label.getLocalBounds();
    label.setOrigin(lb.left + lb.width / 2.f, lb.top + lb.height / 2.f);
    label.setPosition(center.x + offset_x, center.y - 1.f);
    window.draw(label);
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
