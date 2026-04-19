#include "Board.h"
#include "../../src/json.hpp"
#include <algorithm>
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
        if (static_cast<int>(tiles_.size()) != kBoardSize) {
            init_default();
        }
    } catch (...) {
        init_default();
    }
}

void Board::restore_from_save(const nlohmann::json& board_json) {
    if (!board_json.contains("tiles") || !board_json["tiles"].is_array()) {
        init_default();
        return;
    }

    std::vector<Tile> loaded;
    loaded.reserve(kBoardSize);
    for (const auto& t : board_json["tiles"]) {
        int idx = t.value("index", 0);
        TileType type = static_cast<TileType>(t.value("type", 1));
        int price = t.value("price", 0);
        loaded.emplace_back(idx, type, price);
        Tile& tile = loaded.back();
        int owner = t.value("owner_id", -1);
        tile.set_owner(owner >= 0 ? owner : -1);
        tile.set_property_level(static_cast<PropertyLevel>(t.value("level", 0)));
        int g = t.value("guardian_idx", -1);
        if (g >= 0) {
            tile.set_guardian(g);
        } else {
            tile.clear_guardian();
        }
    }

    if (static_cast<int>(loaded.size()) != kBoardSize) {
        init_default();
        return;
    }

    std::sort(loaded.begin(), loaded.end(),
              [](const Tile& a, const Tile& b) { return a.index() < b.index(); });
    for (int i = 0; i < kBoardSize; ++i) {
        if (loaded[static_cast<size_t>(i)].index() != i) {
            init_default();
            return;
        }
    }
    tiles_ = std::move(loaded);
}
