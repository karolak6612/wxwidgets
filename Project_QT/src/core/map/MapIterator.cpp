#include "MapIterator.h"
#include "BaseMap.h" // Only for potential future BaseMap* constructor, not strictly needed now
#include <QDebug>    // For logging

namespace RME {

MapIterator::MapIterator()
    : currentFloorPtr(nullptr), currentFloorX(0), currentFloorY(0),
      currentTile(nullptr), rootNode_ptr_(nullptr), mapMinZ_(0), mapMaxZActual_(0) {}

MapIterator::MapIterator(QTreeNode* rootNode, int mapMinZ, int mapMaxZAbs)
    : currentFloorPtr(nullptr), currentFloorX(0), currentFloorY(0),
      currentTile(nullptr), rootNode_ptr_(rootNode),
      mapMinZ_(mapMinZ), mapMaxZActual_(mapMaxZAbs) {
    if (rootNode_ptr_ && !rootNode_ptr_->isEmpty()) {
        NodeVisitState initialState;
        initialState.node = rootNode_ptr_;
        initialState.childIndex = 0;
        // Floor iterators will be initialized when this node is processed if it's a MAX_DEPTH leaf
        nodeStack.push(initialState);
        findNextTile();
    } else {
        this->rootNode_ptr_ = nullptr; // Mark as end for empty/null root
    }
}

MapIterator& MapIterator::operator++() {
    findNextTile();
    return *this;
}

MapIterator MapIterator::operator++(int) {
    MapIterator temp = *this;
    ++(*this);
    return temp;
}

MapIterator::pointer MapIterator::operator->() const {
    return currentTile;
}

MapIterator::reference MapIterator::operator*() const {
    // Dereferencing an end iterator is undefined behavior.
    // Q_ASSERT(currentTile != nullptr); // Good practice for debugging
    return *currentTile;
}

bool MapIterator::operator==(const MapIterator& other) const {
    // Both iterators are considered "end" if their currentTile is nullptr AND
    // their internal state indicates no more elements (empty stack, or specific end state).
    // The rootNode_ptr_ helps distinguish iterators from different maps.
    if (currentTile == nullptr && other.currentTile == nullptr) {
        // If both were initialized with the same rootNode (or both null for default end iterators),
        // and both stacks are empty, they are equal "end" states.
        bool thisIsExhausted = nodeStack.empty();
        bool otherIsExhausted = other.nodeStack.empty();

        // If both iterators had a root node context initially and are now exhausted
        if (rootNode_ptr_ == other.rootNode_ptr_ && thisIsExhausted && otherIsExhausted) {
            return true;
        }
        // If one is a default-constructed end iterator (rootNode_ptr_ == nullptr)
        // and the other is an exhausted iterator from a map.
        if ((rootNode_ptr_ == nullptr && otherIsExhausted) || (other.rootNode_ptr_ == nullptr && thisIsExhausted)) {
            return true;
        }
        // If both are default-constructed end iterators
        if (rootNode_ptr_ == nullptr && other.rootNode_ptr_ == nullptr) {
             return true;
        }
    }
    // If not both end iterators, compare by the tile they point to.
    // Also ensure they belong to the same conceptual sequence (same root).
    return currentTile == other.currentTile && rootNode_ptr_ == other.rootNode_ptr_;
}

bool MapIterator::operator!=(const MapIterator& other) const {
    return !(*this == other);
}

void MapIterator::findNextTile() {
    currentTile = nullptr;

    while (!nodeStack.empty()) {
        NodeVisitState& currentState = nodeStack.top();
        QTreeNode* node = currentState.node;

        if (node->isLeaf() && node->depth == QTreeNode::MAX_DEPTH) {
            if (!currentState.floorIterationStarted) { // Initialize floor iterators for this leaf
                currentState.floorIterator = node->z_level_floors.constBegin();
                currentState.floorEndIterator = node->z_level_floors.constEnd();
                currentState.floorIterationStarted = true;
                currentFloorPtr = nullptr; // Reset floor pointer for new leaf
                currentFloorY = 0; // Reset Y for the first floor of this leaf
                currentFloorX = -1; // Will be incremented to 0
            }

            // Try to advance within the current floor or find the next valid floor
            if (currentFloorPtr) {
                currentFloorX++;
                if (currentFloorX >= SECTOR_WIDTH_TILES) {
                    currentFloorX = 0;
                    currentFloorY++;
                    if (currentFloorY >= SECTOR_HEIGHT_TILES) {
                        currentFloorPtr = nullptr; // Finished this floor
                    }
                }
            }

            // If currentFloorPtr is null, find the next non-empty floor in Z range
            while (!currentFloorPtr && currentState.floorIterator != currentState.floorEndIterator) {
                int floorZ = currentState.floorIterator.key();
                if (floorZ >= mapMinZ_ && floorZ <= mapMaxZActual_) { // mapMaxZActual_ is inclusive max Z
                     Floor* potentialFloor = currentState.floorIterator.value().get();
                     if (potentialFloor && !potentialFloor->isEmpty()) {
                         currentFloorPtr = potentialFloor;
                         currentFloorX = 0; // Start at (0,0) of this new floor
                         currentFloorY = 0;
                         break; // Found a floor to process
                     }
                }
                ++(currentState.floorIterator);
            }

            // If iterating a valid floor, try to find a tile
            if (currentFloorPtr) {
                // Loop from current (X,Y) to find next tile in this floor
                while (currentFloorY < SECTOR_HEIGHT_TILES) {
                    while (currentFloorX < SECTOR_WIDTH_TILES) {
                        currentTile = currentFloorPtr->getTile(currentFloorX, currentFloorY);
                        if (currentTile) {
                            return; // Found a tile
                        }
                        currentFloorX++;
                    }
                    currentFloorX = 0;
                    currentFloorY++;
                }
                currentFloorPtr = nullptr; // Finished this floor if loop completes
            }

            // If all floors in this leaf are processed, or no valid floor was found
            if (!currentFloorPtr) {
                 nodeStack.pop(); // Done with this leaf node
            }
        } else if (!node->isLeaf()) { // Branch node
            if (currentState.childIndex < 4) {
                QTreeNode* child = node->children[currentState.childIndex].get();
                currentState.childIndex++;
                if (child && !child->isEmpty()) {
                    NodeVisitState nextState;
                    nextState.node = child;
                    // Floor iterators for child will be set when it's processed if it's a MAX_DEPTH leaf
                    nodeStack.push(nextState);
                    // Continue to process this new top of stack (the child)
                }
            } else {
                nodeStack.pop(); // All children of this branch processed
            }
        } else { // Leaf node not at MAX_DEPTH (should be empty or have become a branch)
            nodeStack.pop();
        }
    }
    // If stack is empty, currentTile remains null, indicating end of iteration.
    // For equality check with default-constructed end iterator:
    if (nodeStack.empty() && currentTile == nullptr) {
         // rootNode_ptr_ is kept to identify which map this exhausted iterator belonged to,
         // unless it was default constructed, in which case it's already nullptr.
    }
}

} // namespace RME
