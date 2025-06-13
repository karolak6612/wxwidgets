#ifndef RME_TILEDATA_H
#define RME_TILEDATA_H

#include "core/Position.h"
#include "core/Item.h"      // For std::unique_ptr<Item>
#include "core/Spawn.h"     // For std::unique_ptr<Spawn>
#include "core/Creature.h"  // For std::unique_ptr<Creature>
#include <QList>
#include <memory> // For std::unique_ptr

// Forward declaration
namespace RME {
namespace core {
    class Tile; // Full definition needed for fromTile/applyToTile implementation (in .cpp or if inline)
}
}

namespace RME {
namespace core {
namespace data_transfer {

struct TileData {
    Position position; // Absolute position of this tile data. RelativePosition might be for multi-tile structures.
    std::unique_ptr<Item> ground = nullptr;
    QList<std::unique_ptr<Item>> items;
    std::unique_ptr<Spawn> spawn = nullptr;
    std::unique_ptr<Creature> creature = nullptr;
    int waypointCount = 0; // Added

    TileData() = default; // Default constructor
    TileData(const Position& pos) : position(pos) {} // Constructor with position

    // Rule of five for managing resources if needed, or rely on unique_ptr defaults
    TileData(TileData&& other) = default;
    TileData& operator=(TileData&& other) = default;
    // Deep copy constructor
    TileData(const TileData& other);
    // Deep copy assignment operator
    TileData& operator=(const TileData& other);

    // Factory method to create TileData from a live Tile
    static TileData fromTile(const RME::core::Tile* tile);

    // Method to apply this TileData to a live Tile
    void applyToTile(RME::core::Tile* targetTile) const;

    // Helper to check if it's essentially empty (no ground, items, spawn, creature)
    bool isEmpty() const {
        return !ground && items.isEmpty() && !spawn && !creature && waypointCount == 0;
    }
};

} // namespace data_transfer
} // namespace core
} // namespace RME

#endif // RME_TILEDATA_H
