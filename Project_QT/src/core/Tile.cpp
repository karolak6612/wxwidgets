#include "Tile.h"
#include <algorithm> // For std::find_if

namespace RME {

Tile::Tile(const Position& pos, IItemTypeProvider* provider)
    : position(pos), house_id(0), itemTypeProvider(provider) {
    if (!itemTypeProvider) {
        // Handle missing provider, throw or log as per policy
    }
}

Tile::Tile(int x, int y, int z, IItemTypeProvider* provider)
    : position(x, y, z), house_id(0), itemTypeProvider(provider) {
    if (!itemTypeProvider) {
        // Handle missing provider
    }
}

std::unique_ptr<Tile> Tile::deepCopy() const {
    auto newTile = std::make_unique<Tile>(this->position, this->itemTypeProvider);
    copyMembersTo(*newTile);
    return newTile;
}

void Tile::copyMembersTo(Tile& target) const {
    // Primitives
    target.house_id = this->house_id;
    target.mapFlags = this->mapFlags;
    target.stateFlags = this->stateFlags;
    // itemTypeProvider is set by constructor

    // Smart pointers
    if (this->ground) {
        target.ground = this->ground->deepCopy();
    }
    target.items.clear();
    for (const auto& item : this->items) {
        if (item) {
            target.items.append(item->deepCopy());
        }
    }
    if (this->creature) {
        target.creature = this->creature->deepCopy();
    }
    if (this->spawn) {
        target.spawn = this->spawn->deepCopy();
    }
}

Item* Tile::addItem(std::unique_ptr<Item> item) {
    if (!item || !itemTypeProvider) {
        return nullptr; // Or throw
    }

    Item* rawItemPtr = item.get(); // Get raw pointer before std::move

    // Original RME logic for addItem:
    // 1. If item is ground: replace existing ground.
    // 2. If item has a ground equivalent (property): replace ground, add item to top items.
    // 3. If item is "always on bottom": insert at specific order in top items.
    // 4. Otherwise: append to top items.

    if (itemTypeProvider->isGround(item->getID())) {
        // If it's a ground item, it replaces the current ground
        if (ground) {
            // Consider what to do with the old ground. For now, it's just replaced.
            // If it needs to be returned or moved to 'items', that logic would go here.
        }
        ground = std::move(item);
    } else {
        // Not a ground item, add to the 'items' list
        // Logic for "always on bottom" or specific stacking based on item flags
        // would be implemented here. For now, just append.
        // Example: if (itemTypeProvider->isAlwaysOnBottom(item->getID())) { ... insert ... }
        items.append(std::move(item));
    }

    update(); // Update tile state flags after adding an item
    return rawItemPtr;
}

void Tile::removeItem(Item* itemToRemove) {
    if (!itemToRemove) return;

    if (ground.get() == itemToRemove) {
        ground.reset();
    } else {
        auto it = std::remove_if(items.begin(), items.end(),
                                 [&](const std::unique_ptr<Item>& p) { return p.get() == itemToRemove; });
        if (it != items.end()) {
            items.erase(it, items.end()); // Erase all matching elements (should be one)
        }
    }
    update();
}

std::unique_ptr<Item> Tile::popItem(Item* itemToPop) {
    if (!itemToPop) return nullptr;

    if (ground.get() == itemToPop) {
        update();
        return std::move(ground);
    } else {
        for (int i = 0; i < items.size(); ++i) {
            if (items[i].get() == itemToPop) {
                std::unique_ptr<Item> poppedItem = std::move(items[i]);
                items.removeAt(i);
                update();
                return poppedItem;
            }
        }
    }
    return nullptr;
}


QList<Item*> Tile::getAllItems() const {
    QList<Item*> all;
    if (ground) {
        all.append(ground.get());
    }
    for (const auto& itemPtr : items) {
        all.append(itemPtr.get());
    }
    return all;
}

Item* Tile::getTopItem() const {
    if (!items.isEmpty()) {
        // This should ideally return the topmost *visible* item,
        // considering stacking order and item properties (like always on top).
        // For now, just returns the last item in the list.
        return items.last().get();
    }
    return ground.get(); // If no top items, ground is effectively the top.
}

Item* Tile::getItemAtStackpos(int stackpos) const {
    if (stackpos == 0) {
        return ground.get();
    }
    if (stackpos > 0 && stackpos <= items.size()) {
        return items.at(stackpos - 1).get();
    }
    return nullptr;
}

// Creature Management
void Tile::setCreature(std::unique_ptr<Creature> newCreature) {
    creature = std::move(newCreature);
    update();
}

std::unique_ptr<Creature> Tile::popCreature() {
    update(); // Creature might affect tile state
    return std::move(creature);
}

// Spawn Management
void Tile::setSpawn(std::unique_ptr<Spawn> newSpawn) {
    spawn = std::move(newSpawn);
    update();
}

std::unique_ptr<Spawn> Tile::popSpawn() {
    update(); // Spawn might affect tile state
    return std::move(spawn);
}

bool Tile::isBlocking() const {
    if (stateFlags.testFlag(TileStateFlag::BLOCKING)) { // Check cached flag first
        return true;
    }
    // Detailed check if not cached (or to compute cache)
    if (itemTypeProvider) { // Ensure provider is available
        if (ground && itemTypeProvider->isBlocking(ground->getID())) return true;
        for (const auto& item : items) {
            if (item && itemTypeProvider->isBlocking(item->getID())) return true;
        }
    }
    if (creature) { /* check creature blocking status */ }
    return false;
}

void Tile::update() {
    // Recalculate internal state flags based on current contents.
    bool newBlockingState = false;
    bool newHasTableState = false;
    bool newHasWalkableGround = false;

    if (itemTypeProvider) {
        if (ground) {
            if (itemTypeProvider->isBlocking(ground->getID())) newBlockingState = true;
            // Check for table-like properties if needed for HAS_TABLE
            // if (itemTypeProvider->isTable(ground->getID())) newHasTableState = true;
            if (itemTypeProvider->isWalkable(ground->getID())) newHasWalkableGround = true;

        }
        for (const auto& item : items) {
            if (item) {
                if (itemTypeProvider->isBlocking(item->getID())) newBlockingState = true;
                // if (itemTypeProvider->isTable(item->getID())) newHasTableState = true;
            }
        }
    }
    // Creatures can also block
    if (creature /* && creature->isBlocking() */) { // Assuming creature has isBlocking
        newBlockingState = true;
    }

    if (newBlockingState) addStateFlag(TileStateFlag::BLOCKING);
    else removeStateFlag(TileStateFlag::BLOCKING);

    if (newHasTableState) addStateFlag(TileStateFlag::HAS_TABLE);
    else removeStateFlag(TileStateFlag::HAS_TABLE);

    if (newHasWalkableGround) addStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);
    else removeStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);

    // Other flags like TILESTATE_SELECTED, TILESTATE_MODIFIED are typically set externally.
}

/**
 * @brief Estimates the memory usage of this Tile object.
 *
 * Calculates the total memory by summing the size of the Tile object itself,
 * and the estimated memory usage of its constituent parts: ground item,
 * other items, creature, and spawn.
 *
 * @return size_t Estimated memory usage in bytes.
 */
size_t Tile::estimateMemoryUsage() const {
    size_t memory = sizeof(Tile);

    if (ground) {
        memory += ground->estimateMemoryUsage();
    }

    // Estimate for the QList structure itself plus items
    memory += items.capacity() * sizeof(std::unique_ptr<Item>); // Capacity of the list pointers
    for (const auto& item : items) {
        if (item) {
            memory += item->estimateMemoryUsage();
        }
    }

    if (creature) {
        // Assuming Creature class will have estimateMemoryUsage()
        // For now, using sizeof(Creature) or a placeholder
        // memory += creature->estimateMemoryUsage(); // Ideal
        memory += sizeof(Creature) + 100; // Placeholder for Creature + typical dynamic data
    }

    if (spawn) {
        // Assuming Spawn class will have estimateMemoryUsage()
        // For now, using sizeof(Spawn) or a placeholder
        // memory += spawn->estimateMemoryUsage(); // Ideal
        memory += sizeof(Spawn) + 50; // Placeholder for Spawn + typical dynamic data
    }

    // Add cost of QList<std::unique_ptr<Item>> items itself (beyond capacity, the object overhead)
    // This is often minor but can be included. sizeof(items) would give this.
    // memory += sizeof(items); // Overhead of the QList object itself

    return memory;
}

} // namespace RME
