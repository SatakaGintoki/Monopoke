#include "Tile.h"

Tile::Tile(int index, TileType type, int price)
    : index_(index), type_(type), price_(price) {}

int Tile::toll() const {
    if (!has_owner()) return 0;

    int base = kPropertyTolls[static_cast<int>(level_)];
    float multiplier = kPropertyMultipliers[static_cast<int>(level_)];
    float guardian_factor = has_guardian() ? 1.0f : 0.5f;

    return static_cast<int>(base * multiplier * guardian_factor);
}

int Tile::upgrade_cost() const {
    if (!has_owner() || level_ == PropertyLevel::Gym) return 0;
    return kPropertyBasePrices[static_cast<int>(level_) + 1];
}
