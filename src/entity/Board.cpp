#include "Board.h"
#include "../../src/json.hpp"
#include <fstream>

Board::Board() {
    tiles_.reserve(kBoardSize);
}

Tile& Board::tile_at(int index) {
    return tiles_.at(index);
}

const Tile& Board::tile_at(int index) const {
    return tiles_.at(index);
}

int Board::forward(int from, int steps) const {
    int pos = from;
    for (int i = 0; i < steps; ++i) {
        pos = (pos + 1) % kBoardSize;
    }
    return pos;
}

void Board::init_default() {
    tiles_.clear();

    // 20格默认配置（来自设计文档）
    struct DefaultTile {
        TileType type;
        int price;
    };

    const DefaultTile defaults[kBoardSize] = {
        {TileType::Start,   0},   // 0  起点
        {TileType::Empty, 200},   // 1  空地
        {TileType::Wild,   0},    // 2  野怪格
        {TileType::Empty, 250},    // 3  空地
        {TileType::Shop,   0},    // 4  商店
        {TileType::Empty, 300},   // 5  空地
        {TileType::Wild,   0},    // 6  野怪格
        {TileType::Event,  0},    // 7  事件格
        {TileType::Empty, 300},   // 8  空地
        {TileType::Camp,   0},    // 9  营地
        {TileType::Empty, 350},   // 10 空地
        {TileType::Wild,   0},    // 11 野怪格
        {TileType::Empty, 350},   // 12 空地
        {TileType::Shop,   0},    // 13 商店
        {TileType::Empty, 400},   // 14 空地
        {TileType::Event,  0},    // 15 事件格
        {TileType::Empty, 400},   // 16 空地
        {TileType::Wild,   0},    // 17 野怪格
        {TileType::Empty, 450},   // 18 空地
        {TileType::Battle, 0},    // 19 对战格
    };

    for (int i = 0; i < kBoardSize; ++i) {
        tiles_.emplace_back(i, defaults[i].type, defaults[i].price);
    }
}

void Board::init_from_json(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        init_default();
        return;
    }

    try {
        auto data = nlohmann::json::parse(file);
        tiles_.clear();

        for (const auto& t : data["tiles"]) {
            int idx = t.value("index", 0);
            TileType type = TileType(t.value("type", 1));
            int price = t.value("price", 0);
            tiles_.emplace_back(idx, type, price);
        }
    } catch (...) {
        init_default();
    }
}
