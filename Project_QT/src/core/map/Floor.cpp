#include "Floor.h"
#include <QDebug> // For potential warnings

namespace RME {

Floor::Floor(int z, AssetManager* am)
    : zLevel(z), assetManager(am) {
    tiles.resize(SECTOR_WIDTH_TILES * SECTOR_HEIGHT_TILES);
}

bool Floor::isCoordValid(int localX, int localY) const {
    return localX >= 0 && localX < SECTOR_WIDTH_TILES &&
           localY >= 0 && localY < SECTOR_HEIGHT_TILES;
}

Tile* Floor::getTile(int localX, int localY) {
    if (!isCoordValid(localX, localY)) {
        return nullptr;
    }
    int index = localY * SECTOR_WIDTH_TILES + localX;
    // Basic bounds check, though isCoordValid should cover it.
    if (index < 0 || index >= tiles.size()) return nullptr;
    return tiles[index].get();
}

const Tile* Floor::getTile(int localX, int localY) const {
    if (!isCoordValid(localX, localY)) {
        return nullptr;
    }
    int index = localY * SECTOR_WIDTH_TILES + localX;
    if (index < 0 || index >= tiles.size()) return nullptr;
    return tiles[index].get();
}

Tile* Floor::getOrCreateTile(int localX, int localY, bool& created, const Position& globalPositionForNewTile) {
    created = false;
    if (!isCoordValid(localX, localY)) {
        qWarning() << "Floor::getOrCreateTile - Invalid local coordinates:" << localX << "," << localY << "for Z:" << zLevel;
        return nullptr;
    }
    if (!assetManager) {
         qWarning() << "Floor::getOrCreateTile - AssetManager is null. Cannot create tiles for Z:" << zLevel;
         return nullptr;
    }

    int index = localY * SECTOR_WIDTH_TILES + localX;
    if (index < 0 || index >= tiles.size()) {
        qWarning() << "Floor::getOrCreateTile - Calculated index out of bounds:" << index;
        return nullptr;
    }

    if (!tiles[index]) {
        // Use the provided globalPositionForNewTile for the new Tile.
        // Ensure the Z coordinate matches the floor's Z level.
        if (globalPositionForNewTile.z != zLevel) {
            qWarning() << "Floor::getOrCreateTile - Mismatch between floor Z (" << zLevel
                       << ") and provided globalPositionForNewTile Z (" << globalPositionForNewTile.z << "). Using floor's Z.";
            Position correctedPos(globalPositionForNewTile.x, globalPositionForNewTile.y, zLevel);
            tiles[index] = std::make_unique<Tile>(correctedPos, assetManager);
        } else {
            tiles[index] = std::make_unique<Tile>(globalPositionForNewTile, assetManager);
        }
        created = true;
    }
    return tiles[index].get();
}

bool Floor::removeTile(int localX, int localY) {
    if (!isCoordValid(localX, localY)) {
        return false;
    }
    int index = localY * SECTOR_WIDTH_TILES + localX;
    if (index < 0 || index >= tiles.size()) return false;

    if (tiles[index]) {
        tiles[index].reset();
        return true;
    }
    return false;
}

bool Floor::isEmpty() const {
    for (const auto& tilePtr : tiles) {
        if (tilePtr) {
            return false;
        }
    }
    return true;
}

} // namespace RME
