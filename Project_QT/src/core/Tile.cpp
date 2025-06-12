#include "Tile.h"
#include <algorithm> // For std::remove_if

namespace RME {

Tile::Tile(const Position& pos, IItemTypeProvider* provider)
    : position(pos),
      ground(nullptr),
      creature(nullptr),
      spawn(nullptr),
      m_houseId(0), // Renamed
      m_isHouseExit(false), // Added
      itemTypeProvider(provider),
      m_waypointCount(0)
{
    if (!itemTypeProvider) {
        // This is a critical issue. Consider throwing an exception or logging a fatal error.
        // For now, this might lead to crashes if methods relying on itemTypeProvider are called.
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

Tile::Tile(int x, int y, int z, IItemTypeProvider* provider)
    : position(x, y, z),
      ground(nullptr),
      creature(nullptr),
      spawn(nullptr),
      m_houseId(0), // Renamed
      m_isHouseExit(false), // Added
      itemTypeProvider(provider),
      m_waypointCount(0)
{
    if (!itemTypeProvider) {
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

// Copy constructor
Tile::Tile(const Tile& other)
    : position(other.position),
      m_houseId(other.m_houseId), // Renamed
      m_isHouseExit(other.m_isHouseExit), // Added
      mapFlags(other.mapFlags),
      stateFlags(other.stateFlags),
      itemTypeProvider(other.itemTypeProvider), // Copy non-owning pointer
      m_waypointCount(other.m_waypointCount)
{
    copyMembersTo(*this); // Use helper to deep copy owning members
}

// Copy assignment operator
Tile& Tile::operator=(const Tile& other) {
    if (this == &other) {
        return *this;
    }
    position = other.position;
    m_houseId = other.m_houseId; // Renamed
    m_isHouseExit = other.m_isHouseExit; // Added
    mapFlags = other.mapFlags;
    stateFlags = other.stateFlags;
    itemTypeProvider = other.itemTypeProvider; // Copy non-owning pointer
    m_waypointCount = other.m_waypointCount;

    // Clear existing owned members before copying new ones
    ground.reset();
    items.clear();
    creature.reset();
    spawn.reset();

    copyMembersTo(*this); // Use helper to deep copy owning members
    return *this;
}

// Move constructor
Tile::Tile(Tile&& other) noexcept
    : position(std::move(other.position)),
      ground(std::move(other.ground)),
      items(std::move(other.items)),
      creature(std::move(other.creature)),
      spawn(std::move(other.spawn)),
      m_houseId(other.m_houseId), // Renamed
      m_isHouseExit(other.m_isHouseExit), // Added
      mapFlags(other.mapFlags),
      stateFlags(other.stateFlags),
      itemTypeProvider(other.itemTypeProvider),
      m_waypointCount(other.m_waypointCount)
{
    // Reset simple types in source to a valid state if necessary, though for int/enum it's not critical
    other.m_houseId = 0; // Renamed
    other.m_isHouseExit = false; // Added
    other.mapFlags = TileMapFlag::NO_FLAGS;
    other.stateFlags = TileStateFlag::NO_FLAGS;
    other.itemTypeProvider = nullptr; // Source should not use this after move
    other.m_waypointCount = 0;
}

// Move assignment operator
Tile& Tile::operator=(Tile&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    position = std::move(other.position);
    ground = std::move(other.ground);
    items = std::move(other.items);
    creature = std::move(other.creature);
    spawn = std::move(other.spawn);
    m_houseId = other.m_houseId; // Renamed
    m_isHouseExit = other.m_isHouseExit; // Added
    mapFlags = other.mapFlags;
    stateFlags = other.stateFlags;
    itemTypeProvider = other.itemTypeProvider;
    m_waypointCount = other.m_waypointCount;

    other.m_houseId = 0; // Renamed
    other.m_isHouseExit = false; // Added
    other.mapFlags = TileMapFlag::NO_FLAGS;
    other.stateFlags = TileStateFlag::NO_FLAGS;
    other.itemTypeProvider = nullptr;
    other.m_waypointCount = 0;
    return *this;
}


std::unique_ptr<Tile> Tile::deepCopy() const {
    // Constructor now takes itemTypeProvider
    auto newTile = std::make_unique<Tile>(this->position, this->itemTypeProvider);
    copyMembersTo(*newTile); // Call the existing helper
    return newTile;
}

// copyMembersTo is now primarily for deep-copying unique_ptr members
void Tile::copyMembersTo(Tile& target) const {
    // Primitives and non-owning pointers are copied by Tile's own copy constructor/assignment
    // This helper focuses on heap-allocated members owned by unique_ptr.
    if (this->ground) {
        target.ground = this->ground->deepCopy();
    }
    target.items.clear(); // Ensure target items list is empty before adding copies
    for (const auto& item : this->items) {
        if (item) {
            target.items.append(item->deepCopy());
        }
    }
    if (this->creature) {
        target.creature = this->creature->deepCopy();
    }
    if (this->spawn) {
        target.spawn = this->spawn->deepCopy(); // Assuming Spawn has deepCopy
    }
}

Item* Tile::addItem(std::unique_ptr<Item> item) {
    if (!item || !itemTypeProvider) {
        return nullptr;
    }
    Item* rawItemPtr = item.get();
    if (itemTypeProvider->isGround(item->getID())) {
        ground = std::move(item);
    } else {
        items.append(std::move(item));
    }
    update();
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
            items.erase(it, items.end());
        }
    }
    update();
}

std::unique_ptr<Item> Tile::popItem(Item* itemToPop) {
    if (!itemToPop) return nullptr;
    if (ground.get() == itemToPop) {
        update();
        return std::move(ground);
    }
    for (int i = 0; i < items.size(); ++i) {
        if (items[i].get() == itemToPop) {
            std::unique_ptr<Item> poppedItem = std::move(items[i]);
            items.removeAt(i);
            update();
            return poppedItem;
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
        return items.last().get();
    }
    return ground.get();
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

bool Tile::hasItemOfType(uint16_t id) const {
    if (ground && ground->getID() == id) return true;
    for (const auto& item : items) {
        if (item && item->getID() == id) return true;
    }
    return false;
}

Item* Tile::getItemById(uint16_t id) const {
    if (ground && ground->getID() == id) return ground.get();
    for (const auto& item : items) {
        if (item && item->getID() == id) return item.get();
    }
    return nullptr;
}

void Tile::setGround(std::unique_ptr<Item> newGround) {
    // Ensure the new ground item, if not null, is actually a ground item type.
    // This check might be optional here if Item::create and brush logic already ensure this.
    if (newGround && itemTypeProvider && !itemTypeProvider->isGround(newGround->getID())) {
        // qWarning("Tile::setGround: Attempted to set a non-ground item type (ID: %d) as ground.", newGround->getID());
        // Decide on behavior: reject, or allow and let visual/logic issues occur elsewhere.
        // For now, allow, but this could be stricter.
    }
    ground = std::move(newGround);
    update(); // Call update to refresh tile state (e.g., blocking, walkable ground flags)
}

bool Tile::removeItemById(uint16_t itemId) {
    // Check ground item first
    if (ground && ground->getID() == itemId) {
        ground.reset(); // Clears the ground item
        update();
        return true;
    }

    // Check stacked items
    for (int i = 0; i < items.size(); ++i) {
        if (items[i] && items[i]->getID() == itemId) {
            items.removeAt(i); // Removes the item and shifts subsequent elements
            update();
            return true;
        }
    }
    return false; // Item not found
}

// Creature Management
void Tile::setCreature(std::unique_ptr<RME::core::creatures::Creature> newCreature) {
    creature = std::move(newCreature);
    update();
}

std::unique_ptr<RME::core::creatures::Creature> Tile::popCreature() {
    update();
    return std::move(creature);
}

// Spawn Management
void Tile::setSpawn(std::unique_ptr<Spawn> newSpawn) {
    spawn = std::move(newSpawn);
    update();
}

std::unique_ptr<Spawn> Tile::popSpawn() {
    update();
    return std::move(spawn);
}

bool Tile::isBlocking() const {
    // This is a simplified check. A more accurate one would consult itemTypeProvider for each item.
    // For now, rely on the cached stateFlag if available, or a basic check.
    if (stateFlags.testFlag(TileStateFlag::BLOCKING)) {
        return true;
    }
    // If not cached, perform a basic check (could be expanded)
    if (ground && itemTypeProvider && itemTypeProvider->isBlocking(ground->getID())) return true;
    for (const auto& item : items) {
        if (item && itemTypeProvider && itemTypeProvider->isBlocking(item->getID())) return true;
    }
    if (creature) {
        // Assuming Creature has an isBlocking method or property through its type
        // For now, any creature makes it blocking for simplicity in this update.
        return true;
    }
    return false;
}

void Tile::update() {
    bool newBlockingState = false;
    bool newHasTableState = false; // Example, assuming table logic is more complex
    bool newHasWalkableGround = false;

    if (itemTypeProvider) {
        if (ground) {
            if (itemTypeProvider->isBlocking(ground->getID())) newBlockingState = true;
            if (itemTypeProvider->isWalkable(ground->getID())) newHasWalkableGround = true;
        }
        for (const auto& item : items) {
            if (item && itemTypeProvider->isBlocking(item->getID())) {
                newBlockingState = true;
                break; // If one item blocks, tile blocks
            }
        }
    }

    if (creature) { // If a creature is present, it's generally considered blocking for pathfinding.
        newBlockingState = true;
    }

    if (newBlockingState) addStateFlag(TileStateFlag::BLOCKING);
    else removeStateFlag(TileStateFlag::BLOCKING);

    if (newHasTableState) addStateFlag(TileStateFlag::HAS_TABLE); // Placeholder for table logic
    else removeStateFlag(TileStateFlag::HAS_TABLE);

    if (newHasWalkableGround) addStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);
    else removeStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);

    // Set MODIFIED flag when tile content changes.
    // This should ideally be set by methods that alter content (addItem, removeItem, setCreature, etc.)
    // addStateFlag(TileStateFlag::MODIFIED);
}

size_t Tile::estimateMemoryUsage() const {
    size_t memory = sizeof(Tile);
    if (ground) {
        memory += ground->estimateMemoryUsage();
    }
    memory += items.capacity() * sizeof(std::unique_ptr<Item>);
    for (const auto& item : items) {
        if (item) {
            memory += item->estimateMemoryUsage();
        }
    }
    if (creature) {
        // memory += creature->estimateMemoryUsage(); // Ideal if Creature has this
        memory += sizeof(RME::core::creatures::Creature) + 128; // Approx size for creature + name/outfit data
    }
    if (spawn) {
        // memory += spawn->estimateMemoryUsage(); // Ideal if Spawn has this
        memory += sizeof(Spawn) + 64; // Approx size for spawn + monster list
    }
    return memory;
}

void Tile::increaseWaypointCount() { ++m_waypointCount; }
void Tile::decreaseWaypointCount() { if (m_waypointCount > 0) --m_waypointCount; }
int Tile::getWaypointCount() const { return m_waypointCount; }

// --- House-related Method Implementations ---

// getHouseId() and setHouseId() might already exist if house_id member was present
// Ensure they use m_houseId
uint32_t Tile::getHouseId() const {
    return m_houseId;
}

void Tile::setHouseId(uint32_t houseId) {
    if (m_houseId != houseId) {
        m_houseId = houseId;
        // TODO: If the map is live, mark this tile dirty for rendering/saving
        // if (getMap()) { // Assuming tile has a pointer to its map context via getMap()
        //     getMap()->markTileDirty(getPosition());
        // }
        // addStateFlag(TileStateFlag::MODIFIED); // Or similar mechanism
    }
}

bool Tile::isHouseExit() const {
    return m_isHouseExit;
}

void Tile::setIsHouseExit(bool isExit) {
    if (m_isHouseExit != isExit) {
        m_isHouseExit = isExit;
        // TODO: If the map is live, mark this tile dirty
        // if (getMap()) {
        //     getMap()->markTileDirty(getPosition());
        // }
        // addStateFlag(TileStateFlag::MODIFIED); // Or similar mechanism
    }
}

} // namespace RME
