#pragma once

#include <SFML/Graphics.hpp>
#include "../entity/Board.h"
#include "../entity/Player.h"
#include <vector>
#include <map>

class BoardRenderer {
public:
    BoardRenderer();

    void init(const Board& board);

    void render(sf::RenderWindow& window, const Board& board,
                const Player& p1, const Player& p2);

    sf::Vector2f tile_center(int index) const;

private:
    void calc_positions();
    void load_textures();
    sf::Color tile_color(TileType type) const;
    sf::Color tile_color_dark(TileType type) const;
    std::wstring tile_label(TileType type) const;
    std::wstring tile_icon(TileType type) const;

    void draw_tile(sf::RenderWindow& window, const Tile& tile, int index);
    void draw_player_token(sf::RenderWindow& window, const Player& p,
                           sf::Color color, float offset_x);
    void draw_board_frame(sf::RenderWindow& window);

    sf::Font font_;
    std::vector<sf::Vector2f> positions_;
    
    // 贴图缓存
    std::map<TileType, sf::Texture> tile_textures_;
    sf::Texture player1_texture_;
    sf::Texture player2_texture_;

    static constexpr float kTileW = 180.f;
    static constexpr float kTileH = 140.f;
    static constexpr float kGap = 20.f;
    static constexpr float kBoardX = 0.f;
    static constexpr float kBoardY = 0.f;
};
