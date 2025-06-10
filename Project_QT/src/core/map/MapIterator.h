#ifndef RME_MAP_ITERATOR_H
#define RME_MAP_ITERATOR_H

#include "QTreeNode.h" // Needs full definition for stack
#include "Floor.h"     // Needs full definition for stack
#include "../Tile.h"   // For Tile* return type
#include <stack>
#include <iterator> // For std::forward_iterator_tag (optional, for full compliance)
#include <QMap>     // For QMap iterators in NodeVisitState

namespace RME {

class BaseMap; // Forward declaration

class MapIterator {
public:
    // Iterator traits (optional, for full C++ iterator compliance)
    using iterator_category = std::forward_iterator_tag;
    using value_type = Tile; // Iterates over Tile objects
    using pointer = Tile*;
    using reference = Tile&;
    using difference_type = std::ptrdiff_t;

    // Default constructor for end iterator
    MapIterator();
    // Constructor for begin iterator (used by BaseMap::begin())
    explicit MapIterator(QTreeNode* rootNode, int mapMinZ, int mapMaxZAbs); // mapMaxZAbs is absolute (e.g. 15 for 0-15)
    // explicit MapIterator(BaseMap* map); // Alternative constructor, less direct for now


    // Iterator operations
    MapIterator& operator++();    // Pre-increment
    MapIterator operator++(int); // Post-increment (less efficient)

    pointer operator->() const;
    reference operator*() const;

    bool operator==(const MapIterator& other) const;
    bool operator!=(const MapIterator& other) const;

private:
    void findNextTile(); // Helper to advance to the next valid tile

    // State for iterating through QTreeNodes
    struct NodeVisitState {
        QTreeNode* node = nullptr;
        int childIndex = 0; // Which child of this QTreeNode to visit next (0-3)

        // If node is a leaf at MAX_DEPTH, iterate its z_level_floors
        QMap<int, std::unique_ptr<Floor>>::const_iterator floorIterator;
        QMap<int, std::unique_ptr<Floor>>::const_iterator floorEndIterator;
        bool floorIterationStarted = false; // To initialize iterators only once
    };
    std::stack<NodeVisitState> nodeStack;

    // State for iterating within a Floor
    Floor* currentFloorPtr = nullptr; // Renamed to avoid conflict with class name
    int currentFloorX = 0;
    int currentFloorY = 0;

    int mapMinZ_ = 0;
    int mapMaxZActual_ = 0; // Actual max Z level (e.g., 15 for floors 0-15)

    Tile* currentTile = nullptr;

    // For end iterator comparison and context
    // map_ptr_ removed as rootNode_ptr_ is sufficient for context.
    QTreeNode* rootNode_ptr_ = nullptr; // Store the root to know the map context for end comparison
};

} // namespace RME

#endif // RME_MAP_ITERATOR_H
