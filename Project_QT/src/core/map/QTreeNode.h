#ifndef RME_QTREENODE_H
#define RME_QTREENODE_H

#include "Floor.h" // For std::unique_ptr<Floor>
#include "../Position.h"
#include "../map_constants.h" // For MAP_MAX_WIDTH, etc. for MAX_DEPTH calculation
#include "../../assets/AssetManager.h" // For IItemTypeProvider
#include <array>
#include <memory> // For std::unique_ptr
#include <QMap>   // For z_floors if leaf node stores multiple floors per Z
#include <cmath>  // For std::log2
#include <algorithm> // For std::max

namespace RME {

// Forward declaration
class Tile;

class QTreeNode {
public:
    // Constructor for a node covering a specific area and depth in the tree
    // x,y are top-left corner of this node's area. nodeSize is its width/height.
    QTreeNode(int x, int y, int nodeSize, int depth, AssetManager* assetManager);

    // Children nodes for quadrants (0=NW, 1=NE, 2=SW, 3=SE)
    std::array<std::unique_ptr<QTreeNode>, 4> children;

    // If this is a leaf node AT MAX_DEPTH, it stores floors for different Z levels
    // Key is Z-level.
    QMap<int, std::unique_ptr<Floor>> z_level_floors;

    bool isLeaf() const { return !children[0]; } // If first child is null, it's a leaf

    // Get or create tile at global position.
    // This will traverse or subdivide tree.
    Tile* getOrCreateTile(const Position& pos, bool& created);
    Tile* getTile(const Position& pos); // Read-only access
    const Tile* getTile(const Position& pos) const; // Const version
    bool removeTile(const Position& pos);

    // Recursively removes empty floors and nodes to save memory
    void cleanTree();
    bool isEmpty() const; // Checks if this node (and its children/floors) are all empty

    // For MapIterator or debugging
    int getX() const { return x_coord; }
    int getY() const { return y_coord; }
    int getSize() const { return size; }
    int getDepth() const { return depth; } // Added for debugging/iterator


private:
    void subdivide(); // Creates four children nodes
    int getQuadrant(int targetX, int targetY) const; // Determines which child quadrant (0-3) targetX,Y falls into

    int x_coord, y_coord; // Top-left world coordinates of this node's bounding box
    int size;             // Width and height of this node's bounding box
    int depth;            // Depth in the QuadTree (root is 0)
    AssetManager* assetManager; // Non-owning, passed down for Tile/Floor creation

public: // Made public static const for easier access if needed elsewhere, or keep private
    static const int MAX_DEPTH;
};

} // namespace RME

#endif // RME_QTREENODE_H
