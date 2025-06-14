#include "BaseMap.h"
#include "../map_constants.h"
#include "QTreeNode.h"
#include "MapIterator.h" // Required for method implementations
#include <algorithm>
#include <cmath>
#include <QDebug>

namespace RME {

// Helper to calculate the smallest power of 2 that is >= val
int nextPowerOfTwo(int val) {
    if (val <= 0) return SECTOR_WIDTH_TILES;
    int power = 1;
    while (power < val) {
        power *= 2;
        if (power <= 0) {
            return SECTOR_WIDTH_TILES * static_cast<int>(std::pow(2, QTreeNode::MAX_DEPTH));
        }
    }
    return power;
}

int BaseMap::calculateRootNodeSize(int mapWidth, int mapHeight) {
    int maxDim = std::max(mapWidth, mapHeight);
    if (maxDim <= 0) maxDim = SECTOR_WIDTH_TILES;

    int depthBasedRootSize = SECTOR_WIDTH_TILES * static_cast<int>(std::pow(2.0, static_cast<double>(QTreeNode::MAX_DEPTH)));
    return std::max(nextPowerOfTwo(maxDim), depthBasedRootSize);
}


BaseMap::BaseMap(int mapW, int mapH, int mapF, AssetManager* am)
    : assetManager(am), width(mapW), height(mapH), floors(mapF) {
    if (!assetManager) {
        qCritical("BaseMap: AssetManager cannot be null!");
        return;
    }
    int rootSize = calculateRootNodeSize(width, height);
    rootNode = std::make_unique<QTreeNode>(0, 0, rootSize, 0, assetManager);
    qInfo() << "BaseMap initialized. Dimensions:" << width << "x" << height << "x" << floors
            << ". Root node size:" << rootSize << ". QTree Max depth:" << QTreeNode::MAX_DEPTH;
}

bool BaseMap::isPositionValid(const Position& pos) const {
    return pos.x >= 0 && pos.x < width &&
           pos.y >= 0 && pos.y < height &&
           pos.z >= MAP_MIN_FLOOR && pos.z < floors && pos.z <= MAP_MAX_FLOOR;
}

Tile* BaseMap::getTile(const Position& pos) {
    if (!isPositionValid(pos) || !rootNode) {
        return nullptr;
    }
    return rootNode->getTile(pos);
}

const Tile* BaseMap::getTile(const Position& pos) const {
    if (!isPositionValid(pos) || !rootNode) {
        return nullptr;
    }
    return rootNode->getTile(pos);
}

Tile* BaseMap::getOrCreateTile(const Position& pos, bool& created) {
    created = false;
    if (!isPositionValid(pos)) {
        qWarning() << "BaseMap::getOrCreateTile - Invalid position:" << pos.x << pos.y << pos.z;
        return nullptr;
    }
    if (!rootNode) {
        qWarning() << "BaseMap::getOrCreateTile - Root node is null.";
        return nullptr;
    }
     if (!assetManager) {
        qCritical("BaseMap::getOrCreateTile - AssetManager is null!");
        return nullptr;
    }
    return rootNode->getOrCreateTile(pos, created);
}

bool BaseMap::removeTile(const Position& pos) {
    if (!isPositionValid(pos) || !rootNode) {
        return false;
    }
    return rootNode->removeTile(pos);
}

void BaseMap::setTile(const Position& pos, std::unique_ptr<Tile> newTile) {
    // Allow setting/removing tiles even outside strict map dimensions if root node covers it
    // This is to handle cases where maps might have tiles slightly outside their declared width/height
    // due to sector alignment or previous editor versions.
    // However, creating new tiles should ideally be within valid map dimensions.
    if (!newTile && !isPositionValid(pos)) {
        // If trying to remove a tile from an invalid position,
        // and it's truly outside any reasonable boundary covered by rootNode,
        // then rootNode->setTile will handle it (e.g. by doing nothing if pos is outside root).
        // If strict boundaries are needed for removal too:
        // if (!isPositionValid(pos)) return;
    }

    if (newTile && !isPositionValid(pos)) {
        qWarning("BaseMap::setTile: Attempt to set a new tile at invalid map position (%d,%d,%d).",
                 pos.x, pos.y, pos.z);
        return; // Do not place new tiles outside logical map boundaries.
    }

    if (!rootNode) {
        qWarning("BaseMap::setTile: Root node is null.");
        return;
    }

    // The Tile object, if not null, should have its internal position set correctly by its constructor.
    // When undoing, a deep-copied tile is passed in; its internal position should match 'pos'.
    if (newTile && newTile->getPosition() != pos) {
        // This is a critical check. The tile passed must be for this specific position.
        // A Tile's Position is immutable after construction.
        qCritical("BaseMap::setTile: Mismatch between target Position (%d,%d,%d) and Tile's internal Position (%d,%d,%d). Aborting.",
                  pos.x, pos.y, pos.z,
                  newTile->getPosition().x, newTile->getPosition().y, newTile->getPosition().z);
        // Not setting the tile to prevent potential corruption or hard-to-debug issues.
        // The unique_ptr will be destroyed, releasing the tile.
        return;
    }

    rootNode->setTile(pos, std::move(newTile));
    // After setting a tile, especially to nullptr, the QTreeNode might have become empty
    // and could be pruned. This can be done periodically or after certain operations.
    // rootNode->cleanTree(); // This might be too frequent here.
}

// --- Iterator Methods ---
MapIterator BaseMap::begin() {
    if (!rootNode) {
        return MapIterator(); // End iterator
    }
    // mapMinZ is MAP_MIN_FLOOR, mapMaxZ is this->floors (exclusive upper bound for iterator)
    // Or, if MapIterator expects inclusive max Z: this->floors - 1
    // The MapIterator constructor was defined as: MapIterator(QTreeNode* rootNode, int mapMinZ, int mapMaxZAbs);
    // And its loop is: if (floorZ >= mapMinZ_ && floorZ <= mapMaxZActual_)
    // So, mapMaxZAbs should be this->floors - 1 if floors is a count (e.g. 16 for 0-15).
    int maxZLevelForIterator = (this->floors > 0) ? (this->floors - 1) : -1; // Max Z index
    if (maxZLevelForIterator < MAP_MIN_FLOOR) { // No valid floors to iterate
        return MapIterator(); // Return end iterator
    }
    return MapIterator(rootNode.get(), MAP_MIN_FLOOR, maxZLevelForIterator);
}

MapIterator BaseMap::end() {
    // A default-constructed MapIterator serves as the end iterator.
    // Its rootNode_ptr_ will be nullptr, which operator== handles.
    return MapIterator();
}

} // namespace RME
