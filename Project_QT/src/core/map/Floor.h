#ifndef RME_FLOOR_H
#define RME_FLOOR_H

#include "../Tile.h" // For std::unique_ptr<Tile>
#include "../Position.h"
#include "core/assets/AssetManager.h" // For IItemTypeProvider
#include <QVector>
#include <memory> // For std::unique_ptr

namespace RME {

// Defines the size of a floor sector (e.g., 32x32 tiles)
constexpr int SECTOR_WIDTH_TILES = 32;
constexpr int SECTOR_HEIGHT_TILES = 32;

class Floor {
public:
    Floor(int zLevel, AssetManager* assetManager);

    Tile* getTile(int localX, int localY);
    const Tile* getTile(int localX, int localY) const;

    // Gets or creates a tile at the local coordinates.
    // Sets 'created' to true if a new tile was actually made.
    // globalPositionForNewTile is the absolute world position for the new Tile.
    Tile* getOrCreateTile(int localX, int localY, bool& created, const Position& globalPositionForNewTile);

    bool removeTile(int localX, int localY);
    void setTile(int localX, int localY, std::unique_ptr<Tile> newTile);
    bool isEmpty() const;
    int getZLevel() const { return zLevel; }

private:
    int zLevel;
    AssetManager* assetManager;
    QVector<std::unique_ptr<Tile>> tiles;

    bool isCoordValid(int localX, int localY) const;
};

} // namespace RME

#endif // RME_FLOOR_H
