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

void Floor::setTile(int localX, int localY, std::unique_ptr<Tile> newTile) {
    if (!isCoordValid(localX, localY)) {
        qWarning("Floor::setTile: Invalid local coordinates (%d, %d) for Z: %d", localX, localY, zLevel);
        return;
    }
    int index = localY * SECTOR_WIDTH_TILES + localX;
    // Basic bounds check, though isCoordValid should cover it.
    if (index < 0 || index >= tiles.size()) {
         qWarning("Floor::setTile: Calculated index out of bounds: %d for Z: %d", index, zLevel);
        return;
    }

    if (newTile) {
        // Ensure new tile's internal Z-coordinate is consistent with this floor.
        // The X and Y coordinates are local to the floor sector.
        // The Tile object itself stores its global Position.
        // This consistency check is important.
        if (newTile->getPosition().z != this->zLevel) {
            // This is a significant issue. The tile being placed has a Z coordinate
            // that doesn't match the floor it's being placed on.
            // This could lead to inconsistencies if not handled.
            // For now, we'll log a warning and proceed, but this indicates
            // a potential problem in how tiles are created or moved.
            qWarning("Floor::setTile: Tile Z (%d) does not match Floor Z (%d) at local (%d, %d). Tile's global pos: (%d,%d,%d)",
                     newTile->getPosition().z, this->zLevel, localX, localY,
                     newTile->getPosition().x, newTile->getPosition().y, newTile->getPosition().z);
            // Option: Correct the tile's Z? Or reject? For now, allow but warn.
        }
        tiles[index] = std::move(newTile);
    } else {
        tiles[index].reset(); // Effectively removes the tile
    }
    // No direct notification from Floor, Map will handle it.
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
