#pragma once

#include "Tile.h"
#include <vector>

class Board {
public:
    Board();

    // 获取格子
    Tile& tile_at(int index);
    const Tile& tile_at(int index) const;

    // 获取所有格子
    const std::vector<Tile>& tiles() const { return tiles_; }

    // 获取格子数量
    int size() const { return kBoardSize; }

    // 从某位置向前走N步（环形，超出末尾会绕回）
    int forward(int from, int steps) const;

    // 初始化棋盘数据（从JSON或默认）
    void init_default();
    void init_from_json(const std::string& json_path);

private:
    std::vector<Tile> tiles_;
};
