#include "Tile.h"
#include <algorithm> // For std::remove_if

namespace RME {

Tile::Tile(const Position& pos, IItemTypeProvider* provider)
    : position(pos),
      ground(nullptr),
      creature(nullptr),
      m_spawnDataRef(nullptr), // Initialized new spawn ref
      m_houseId(0),
      m_isHouseExit(false),
      // m_isProtectionZone(false), // Removed
      itemTypeProvider(provider),
      m_waypointCount(0)
      // Old spawn members (m_spawnRadius, m_spawnIntervalSeconds, m_spawnCreatureList) removed
{
    if (!itemTypeProvider) {
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

Tile::Tile(int x, int y, int z, IItemTypeProvider* provider)
    : position(x, y, z),
      ground(nullptr),
      creature(nullptr),
      m_spawnDataRef(nullptr), // Initialized new spawn ref
      m_houseId(0),
      m_isHouseExit(false),
      // m_isProtectionZone(false), // Removed
      itemTypeProvider(provider),
      m_waypointCount(0)
      // Old spawn members (m_spawnRadius, m_spawnIntervalSeconds, m_spawnCreatureList) removed
{
    if (!itemTypeProvider) {
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

// Copy constructor
Tile::Tile(const Tile& other)
    : position(other.position),
      m_spawnDataRef(other.m_spawnDataRef), // Copy spawn data reference
      m_houseId(other.m_houseId),
      m_isHouseExit(other.m_isHouseExit),
      // m_isProtectionZone(other.m_isProtectionZone), // Removed
      mapFlags(other.mapFlags),
      stateFlags(other.stateFlags),
      itemTypeProvider(other.itemTypeProvider),
      m_waypointCount(other.m_waypointCount)
      // Old spawn members (m_spawnRadius, m_spawnCreatureList, m_spawnIntervalSeconds) removed
{
    // Deep copy unique_ptr members
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item : other.items) {
        if (item) items.append(item->deepCopy());
    }
    if (other.creature) creature = other.creature->deepCopy();
}

// Copy assignment operator
Tile& Tile::operator=(const Tile& other) {
    if (this == &other) {
        return *this;
    }
    position = other.position;
    m_spawnDataRef = other.m_spawnDataRef; // Copy spawn data reference
    m_houseId = other.m_houseId;
    m_isHouseExit = other.m_isHouseExit;
    // m_isProtectionZone = other.m_isProtectionZone; // Removed
    mapFlags = other.mapFlags;
    stateFlags = other.stateFlags;
    itemTypeProvider = other.itemTypeProvider; // Copy the pointer
    m_waypointCount = other.m_waypointCount;
    // Old spawn members (m_spawnRadius, m_spawnCreatureList, m_spawnIntervalSeconds) removed

    // Clear existing owned members before copying new ones
    ground.reset();
    items.clear();
    creature.reset();
    // Deep copy unique_ptr members
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item : other.items) {
        if (item) items.append(item->deepCopy());
    }
    if (other.creature) creature = other.creature->deepCopy();
    return *this;
}

// Move constructor
Tile::Tile(Tile&& other) noexcept
    : position(std::move(other.position)),
      ground(std::move(other.ground)),
      items(std::move(other.items)),
      creature(std::move(other.creature)),
      m_spawnDataRef(std::exchange(other.m_spawnDataRef, nullptr)), // Move spawn data reference
      m_houseId(std::exchange(other.m_houseId, 0)),
      m_isHouseExit(std::exchange(other.m_isHouseExit, false)),
      // m_isProtectionZone(std::exchange(other.m_isProtectionZone, false)), // Removed
      mapFlags(std::exchange(other.mapFlags, TileMapFlag::NO_FLAGS)),
      stateFlags(std::exchange(other.stateFlags, TileStateFlag::NO_FLAGS)),
      itemTypeProvider(std::exchange(other.itemTypeProvider, nullptr)),
      m_waypointCount(std::exchange(other.m_waypointCount, 0))
      // Old spawn members (m_spawnRadius, m_spawnCreatureList, m_spawnIntervalSeconds) removed
{
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
    m_spawnDataRef = std::exchange(other.m_spawnDataRef, nullptr); // Move spawn data reference
    m_houseId = std::exchange(other.m_houseId, 0);
    m_isHouseExit = std::exchange(other.m_isHouseExit, false);
    // m_isProtectionZone = std::exchange(other.m_isProtectionZone, false); // Removed
    mapFlags = std::exchange(other.mapFlags, TileMapFlag::NO_FLAGS);
    stateFlags = std::exchange(other.stateFlags, TileStateFlag::NO_FLAGS);
    itemTypeProvider = std::exchange(other.itemTypeProvider, nullptr);
    m_waypointCount = std::exchange(other.m_waypointCount, 0);
    // Old spawn members (m_spawnRadius, m_spawnCreatureList, m_spawnIntervalSeconds) removed

    other.mapFlags = TileMapFlag::NO_FLAGS;
    other.stateFlags = TileStateFlag::NO_FLAGS;
    other.itemTypeProvider = nullptr;
    other.m_waypointCount = 0;
    return *this;
}


std::unique_ptr<Tile> Tile::deepCopy() const {
    // Constructor now takes itemTypeProvider
    auto newTile = std::make_unique<Tile>(this->position, this->itemTypeProvider);

    newTile->m_spawnDataRef = this->m_spawnDataRef; // Copy spawn data reference
    newTile->m_houseId = this->m_houseId;
    newTile->m_isHouseExit = this->m_isHouseExit;
    // newTile->m_isProtectionZone = this->m_isProtectionZone; // Removed
    newTile->mapFlags = this->mapFlags;
    // Be careful with stateFlags, e.g., SELECTED and MODIFIED should usually not be deep copied.
    // For simplicity, copying all, but this might need refinement.
    newTile->stateFlags = this->stateFlags;
    newTile->m_waypointCount = this->m_waypointCount;
    // Old spawn members (m_spawnRadius, m_spawnCreatureList, m_spawnIntervalSeconds) removed.

    // Deep copy unique_ptr members
    if (this->ground) newTile->ground = this->ground->deepCopy();
    newTile->items.clear();
    for (const auto& item : this->items) {
        if (item) newTile->items.append(item->deepCopy());
    }
    if (this->creature) newTile->creature = this->creature->deepCopy();

    return newTile;
}

// copyMembersTo: This function's role needs to be re-evaluated.
// If copy/move constructors and assignments handle all members correctly,
// and deepCopy creates a new object and then copies, this might be redundant
// or only for specific internal uses.
// For now, updating it to reflect new member structure.
void Tile::copyMembersTo(Tile& target) const {
    // This method is less about initial construction and more about updating an existing 'target'
    // It should handle all relevant members.
    target.position = this->position; // Usually position is set at construction, but for completeness
    target.m_spawnDataRef = this->m_spawnDataRef;
    target.m_houseId = this->m_houseId;
    target.m_isHouseExit = this->m_isHouseExit;
    // target.m_isProtectionZone = this->m_isProtectionZone; // Removed
    target.mapFlags = this->mapFlags;
    target.stateFlags = this->stateFlags; // Again, consider which state flags are appropriate to copy
    target.itemTypeProvider = this->itemTypeProvider;
    target.m_waypointCount = this->m_waypointCount;
    // Old spawn members removed

    // Deep copy unique_ptr members
    if (this->ground) {
        target.ground = this->ground->deepCopy();
    } else {
        target.ground.reset();
    }
    target.items.clear();
    for (const auto& item : this->items) {
        if (item) {
            target.items.append(item->deepCopy());
        }
    }
    if (this->creature) {
        target.creature = this->creature->deepCopy();
    } else {
        target.creature.reset();
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

RME::core::Item* RME::core::Tile::getTopItemByID(uint16_t id) const {
    // Iterate from top of stack (end of list) downwards for non-ground items
    for (int i = items.size() - 1; i >= 0; --i) {
        if (items.at(i) && items.at(i)->getID() == id) {
            return items.at(i).get();
        }
    }
    return nullptr; // Not found among non-ground items
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

// New Spawn Data Methods Implementation
bool Tile::isSpawnTile() const {
    return m_spawnDataRef != nullptr;
}

int Tile::getSpawnRadius() const {
    return m_spawnDataRef ? m_spawnDataRef->getRadius() : 0;
}

const QStringList& Tile::getSpawnCreatureList() const {
    static const QStringList emptyList; // Static to return a valid reference
    return m_spawnDataRef ? m_spawnDataRef->getCreatureTypes() : emptyList;
}

int Tile::getSpawnIntervalSeconds() const {
    return m_spawnDataRef ? m_spawnDataRef->getIntervalSeconds() : 0;
}

void Tile::setSpawnDataRef(RME::SpawnData* ref) {
    if (m_spawnDataRef != ref) {
        m_spawnDataRef = ref;
        addStateFlag(TileStateFlag::MODIFIED); // Mark tile as modified
        // Consider if the presence of a spawn affects other tile states (e.g. blocking) and call update()
        update();
    }
}

void Tile::clearSpawnDataRef() {
    if (m_spawnDataRef) {
        m_spawnDataRef = nullptr;
        addStateFlag(TileStateFlag::MODIFIED); // Mark tile as modified
        update();
    }
}

// Selection State Methods
bool Tile::isSelected() const {
    return hasStateFlag(TileStateFlag::SELECTED);
}

void Tile::setSelected(bool selected) {
    if (selected) {
        addStateFlag(TileStateFlag::SELECTED);
    } else {
        removeStateFlag(TileStateFlag::SELECTED);
    }
    // Note: Does not automatically set MODIFIED, selection is often transient editor state.
}

bool Tile::hasSelectedElements() const {
    if (isSelected()) return true;
    if (ground && ground->isSelected()) return true;
    for (const auto& item : items) {
        if (item && item->isSelected()) return true;
    }
    if (creature && creature->isSelected()) return true; // Assumes Creature will have isSelected()
    if (m_spawnDataRef && m_spawnDataRef->isSelected()) return true; // Assumes SpawnData will have isSelected()
    return false;
}


// Updated isEmpty (implementation for declaration in .h)
bool Tile::isEmpty() const {
    // A tile is empty if it has no items, no creature, and no spawn data reference.
    return getItemCount() == 0 && !creature && !m_spawnDataRef;
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

    if (creature) {
        newBlockingState = true; // Creatures always block
    }
    // SpawnDataRef itself doesn't make a tile blocking.
    // If specific items related to a visible spawn point (e.g. a spawn monster type item)
    // were on the tile, they would be handled by the item iteration.

    if (newBlockingState) addStateFlag(TileStateFlag::BLOCKING);
    else removeStateFlag(TileStateFlag::BLOCKING);

    if (newHasTableState) addStateFlag(TileStateFlag::HAS_TABLE); // Placeholder for table logic
    else removeStateFlag(TileStateFlag::HAS_TABLE);

    if (newHasWalkableGround) addStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);
    else removeStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);

    // Set MODIFIED flag when tile content changes.
    // This should ideally be set by methods that alter content (addItem, removeItem, setCreature, etc.)
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
        memory += sizeof(RME::core::creatures::Creature) + 128; // Approx for Creature object itself
    }
    memory += sizeof(m_spawnDataRef); // Add size of the pointer

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

// --- Protection Zone ---
// Removed setIsProtectionZone and isProtectionZone methods that used m_isProtectionZone.
// isPZ() in Tile.h already uses hasMapFlag(TileMapFlag::PROTECTION_ZONE), which is the correct way.

} // namespace RME
