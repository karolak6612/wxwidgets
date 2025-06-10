#include "QTreeNode.h"
#include "Floor.h" // Required for std::unique_ptr<Floor> definition
#include "../Tile.h" // Required for Tile manipulation, Position
#include <QDebug>  // For warnings
#include <cmath>   // For std::log2 in MAX_DEPTH, ensure this is included
#include <algorithm> // For std::max in MAX_DEPTH

namespace RME {

// Initialize MAX_DEPTH
const int QTreeNode::MAX_DEPTH = []{
    int maxMapDim = std::max(MAP_MAX_WIDTH, MAP_MAX_HEIGHT);
    if (SECTOR_WIDTH_TILES <= 0) return 0; // Avoid division by zero or invalid sector size

    // Calculate depth required for nodes to become SECTOR_WIDTH_TILES
    // If maxMapDim is already <= SECTOR_WIDTH_TILES, depth is 0.
    if (maxMapDim <= SECTOR_WIDTH_TILES) return 0;

    int calculatedDepth = 0;
    int currentSize = SECTOR_WIDTH_TILES;
    while(currentSize < maxMapDim) {
        currentSize *= 2;
        calculatedDepth++;
        if (calculatedDepth > 20) { // Safety break for extreme constants or logic error
            qWarning("QTreeNode MAX_DEPTH calculation exceeded 20, check map constants. Defaulting to 10.");
            return 10;
        }
    }
    return calculatedDepth;
}();


QTreeNode::QTreeNode(int x, int y, int nodeSize, int d, AssetManager* am)
    : x_coord(x), y_coord(y), size(nodeSize), depth(d), assetManager(am) {
}

int QTreeNode::getQuadrant(int targetX, int targetY) const {
    int midX = x_coord + size / 2;
    int midY = y_coord + size / 2;
    if (targetX < midX) {
        return (targetY < midY) ? 0 : 2; // NW (0) or SW (2)
    } else {
        return (targetY < midY) ? 1 : 3; // NE (1) or SE (3)
    }
}

void QTreeNode::subdivide() {
    if (!isLeaf() || depth >= MAX_DEPTH) {
        return;
    }
    int childSize = size / 2;
    // It's possible childSize could become less than SECTOR_WIDTH_TILES if MAX_DEPTH is very high
    // or initial map size is not a multiple of SECTOR_WIDTH_TILES * 2^N.
    // The MAX_DEPTH calculation should ideally prevent this.
    // If childSize is 0 (e.g. size was 1), it can't be subdivided.
    if (childSize == 0) {
        qWarning() << "QTreeNode::subdivide: Child size would be 0, cannot subdivide node at depth" << depth << "with size" << size;
        return;
    }


    children[0] = std::make_unique<QTreeNode>(x_coord, y_coord, childSize, depth + 1, assetManager);
    children[1] = std::make_unique<QTreeNode>(x_coord + childSize, y_coord, childSize, depth + 1, assetManager);
    children[2] = std::make_unique<QTreeNode>(x_coord, y_coord + childSize, childSize, depth + 1, assetManager);
    children[3] = std::make_unique<QTreeNode>(x_coord + childSize, y_coord + childSize, childSize, depth + 1, assetManager);

    if (!z_level_floors.isEmpty()) {
        qWarning() << "QTreeNode::subdivide: Node at depth" << depth << "had z_level_floors. These are being discarded!";
        z_level_floors.clear();
    }
}

Tile* QTreeNode::getTile(const Position& pos) {
    return const_cast<Tile*>(static_cast<const QTreeNode*>(this)->getTile(pos));
}

const Tile* QTreeNode::getTile(const Position& pos) const {
    if (pos.x < x_coord || pos.x >= x_coord + size ||
        pos.y < y_coord || pos.y >= y_coord + size) {
        return nullptr;
    }

    if (!isLeaf()) {
        return children[getQuadrant(pos.x, pos.y)]->getTile(pos);
    } else {
        if (depth < MAX_DEPTH) {
            return nullptr;
        }
        auto it = z_level_floors.constFind(pos.z);
        if (it != z_level_floors.constEnd() && it.value()) {
            int localX = pos.x - x_coord;
            int localY = pos.y - y_coord;
            return it.value()->getTile(localX, localY);
        }
        return nullptr;
    }
}

Tile* QTreeNode::getOrCreateTile(const Position& pos, bool& created) {
    created = false;
    if (pos.x < x_coord || pos.x >= x_coord + size ||
        pos.y < y_coord || pos.y >= y_coord + size) {
        qWarning() << "QTreeNode::getOrCreateTile - Position" << pos.x << "," << pos.y << "," << pos.z
                   << "is outside node bounds (" << x_coord << "," << y_coord << "size" << size << "depth" << depth << ")";
        return nullptr;
    }

    if (depth < MAX_DEPTH) {
        if (isLeaf()) {
            subdivide();
            if (isLeaf()) {
                 qCritical() << "QTreeNode: Failed to subdivide node at depth" << depth << "though it's not MAX_DEPTH.";
                 return nullptr;
            }
        }
        return children[getQuadrant(pos.x, pos.y)]->getOrCreateTile(pos, created);
    } else {
        if (!isLeaf()) {
            qCritical() << "QTreeNode: At MAX_DEPTH but is not a leaf node! This indicates a logic error.";
            return nullptr;
        }

        auto it = z_level_floors.find(pos.z);
        if (it == z_level_floors.end() || !it.value()) {
            it = z_level_floors.insert(pos.z, std::make_unique<Floor>(pos.z, assetManager));
        }

        int localX = pos.x - x_coord;
        int localY = pos.y - y_coord;

        // Pass the global 'pos' to Floor::getOrCreateTile
        Tile* tile = it.value()->getOrCreateTile(localX, localY, created, pos);
        // The const_cast hack is no longer needed as Floor now sets the position.
        return tile;
    }
}

bool QTreeNode::removeTile(const Position& pos) {
    if (pos.x < x_coord || pos.x >= x_coord + size ||
        pos.y < y_coord || pos.y >= y_coord + size) {
        return false;
    }

    bool removed = false;
    if (!isLeaf()) {
        removed = children[getQuadrant(pos.x, pos.y)]->removeTile(pos);
    } else {
        if (depth < MAX_DEPTH) return false;

        auto floorIt = z_level_floors.find(pos.z);
        if (floorIt != z_level_floors.end() && floorIt.value()) {
            int localX = pos.x - x_coord;
            int localY = pos.y - y_coord;
            removed = floorIt.value()->removeTile(localX, localY);
            if (removed && floorIt.value()->isEmpty()) {
                z_level_floors.erase(floorIt);
            }
        }
    }

    if (removed) {
        cleanTree();
    }
    return removed;
}

void QTreeNode::cleanTree() {
    if (isLeaf()) {
        return;
    }

    bool allChildrenAreEmptyAndLeaves = true;
    for (int i = 0; i < 4; ++i) {
        if (children[i]) {
            children[i]->cleanTree();
            // A child is suitable for pruning if it's now an empty leaf.
            // Note: isEmpty() for a branch checks its children. A leaf is empty if it has no floors/tiles.
            if (!children[i]->isEmpty()) {
                allChildrenAreEmptyLeaves = false;
            }
        } else {
            // This case should ideally not be hit if !isLeaf() because subdivide creates all 4.
            // However, if it can happen, this child is effectively "empty".
            // But if one child is null, it's not a valid state for a non-leaf node that could be pruned to an empty leaf.
            allChildrenAreEmptyLeaves = false;
        }
    }

    if (allChildrenAreEmptyLeaves) {
        // To become a leaf, all children must not only be empty but also themselves leaves.
        // The isEmpty() check above covers this: an empty branch node has all its children empty.
        // If those children are also leaves, then this node can become a leaf.
        bool canPrune = true;
        for(int i=0; i<4; ++i) {
            if(children[i] && !children[i]->isLeaf()){
                canPrune = false;
                break;
            }
        }
        if(canPrune){
            for (int i = 0; i < 4; ++i) {
                children[i].reset();
            }
        }
    }
}

bool QTreeNode::isEmpty() const {
    if (!isLeaf()) {
        for (int i = 0; i < 4; ++i) {
            if (children[i] && !children[i]->isEmpty()) {
                return false;
            }
        }
        return true;
    } else {
        if (depth < MAX_DEPTH) {
             return z_level_floors.isEmpty();
        }
        if (z_level_floors.isEmpty()) return true;
        for (const auto& pair : z_level_floors) {
            if (pair.second && !pair.second->isEmpty()) {
                return false;
            }
        }
        return true;
    }
}

} // namespace RME
