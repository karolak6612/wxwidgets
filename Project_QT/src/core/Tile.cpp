#include "Tile.h"
#include <algorithm> // For std::remove_if

namespace RME {

Tile::Tile(const Position& pos, IItemTypeProvider* provider)
    : position(pos),
      ground(nullptr),
      creature(nullptr),
      // spawn(nullptr), // Removed old spawn
      // m_spawnDataRef(nullptr), // Removed old spawn ref
      m_houseId(0),
      m_isHouseExit(false),
      m_isProtectionZone(false),
      itemTypeProvider(provider),
      m_waypointCount(0),
      m_spawnRadius(0),                 // Added
      m_spawnIntervalSeconds(0)       // Added
      // m_spawnCreatureList is default-initialized
{
    if (!itemTypeProvider) {
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

Tile::Tile(int x, int y, int z, IItemTypeProvider* provider)
    : position(x, y, z),
      ground(nullptr),
      creature(nullptr),
      // spawn(nullptr), // Removed old spawn
      // m_spawnDataRef(nullptr), // Removed old spawn ref
      m_houseId(0),
      m_isHouseExit(false),
      m_isProtectionZone(false),
      itemTypeProvider(provider),
      m_waypointCount(0),
      m_spawnRadius(0),                 // Added
      m_spawnIntervalSeconds(0)       // Added
      // m_spawnCreatureList is default-initialized
{
    if (!itemTypeProvider) {
        // throw std::runtime_error("Tile created with null IItemTypeProvider");
    }
}

// Copy constructor
Tile::Tile(const Tile& other)
    : position(other.position),
      m_houseId(other.m_houseId),
      m_isHouseExit(other.m_isHouseExit),
      m_isProtectionZone(other.m_isProtectionZone), // Added
      mapFlags(other.mapFlags),
      stateFlags(other.stateFlags),
      itemTypeProvider(other.itemTypeProvider),
      m_waypointCount(other.m_waypointCount),
      m_spawnRadius(other.m_spawnRadius),                 // Added
      m_spawnCreatureList(other.m_spawnCreatureList),     // Added
      m_spawnIntervalSeconds(other.m_spawnIntervalSeconds) // Added
{
    // Deep copy unique_ptr members, assuming copyMembersTo is not used by copy constructor
    // or that copyMembersTo is designed for this.
    // The original copyMembersTo was for deep copying unique_ptrs.
    // Let's call it explicitly for those if it's not part of the constructor's design.
    // For now, assuming the direct member copies are sufficient for non-owning,
    // and unique_ptrs need explicit deep copy logic.
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item : other.items) {
        if (item) items.append(item->deepCopy());
    }
    if (other.creature) creature = other.creature->deepCopy();
    // spawn member removed
    // m_spawnDataRef member removed
}

// Copy assignment operator
Tile& Tile::operator=(const Tile& other) {
    if (this == &other) {
        return *this;
    }
    position = other.position;
    m_houseId = other.m_houseId;
    m_isHouseExit = other.m_isHouseExit;
    m_isProtectionZone = other.m_isProtectionZone;
    mapFlags = other.mapFlags;
    stateFlags = other.stateFlags;
    itemTypeProvider = other.itemTypeProvider; // Copy the pointer
    m_waypointCount = other.m_waypointCount;

    m_spawnRadius = other.m_spawnRadius;                 // Added
    m_spawnCreatureList = other.m_spawnCreatureList;     // Added
    m_spawnIntervalSeconds = other.m_spawnIntervalSeconds; // Added

    // Clear existing owned members before copying new ones
    ground.reset();
    items.clear();
    creature.reset();
    // spawn.reset(); // Removed old spawn

    // Deep copy unique_ptr members
    if (other.ground) ground = other.ground->deepCopy();
    for (const auto& item : other.items) {
        if (item) items.append(item->deepCopy());
    }
    if (other.creature) creature = other.creature->deepCopy();
    // No spawn unique_ptr to copy

    return *this;
}

// Move constructor
Tile::Tile(Tile&& other) noexcept
    : position(std::move(other.position)),
      ground(std::move(other.ground)),
      items(std::move(other.items)),
      creature(std::move(other.creature)),
      // spawn(std::move(other.spawn)), // Removed old spawn
      // m_spawnDataRef(nullptr), // Removed old spawn ref
      m_houseId(std::exchange(other.m_houseId, 0)),
      m_isHouseExit(std::exchange(other.m_isHouseExit, false)),
      m_isProtectionZone(std::exchange(other.m_isProtectionZone, false)),
      mapFlags(std::exchange(other.mapFlags, TileMapFlag::NO_FLAGS)),
      stateFlags(std::exchange(other.stateFlags, TileStateFlag::NO_FLAGS)),
      itemTypeProvider(std::exchange(other.itemTypeProvider, nullptr)),
      m_waypointCount(std::exchange(other.m_waypointCount, 0)),
      m_spawnRadius(std::exchange(other.m_spawnRadius, 0)),                 // Added
      m_spawnCreatureList(std::move(other.m_spawnCreatureList)),           // Added
      m_spawnIntervalSeconds(std::exchange(other.m_spawnIntervalSeconds, 0)) // Added
{
    // Ensure other is in a valid but empty/default state for owning pointers after move.
    // std::exchange handles primitives. std::move handles unique_ptrs and QStringList.
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
    // spawn = std::move(other.spawn); // Removed old spawn
    // m_spawnDataRef = nullptr; // Removed old spawn ref
    m_houseId = std::exchange(other.m_houseId, 0);
    m_isHouseExit = std::exchange(other.m_isHouseExit, false);
    m_isProtectionZone = std::exchange(other.m_isProtectionZone, false);
    mapFlags = std::exchange(other.mapFlags, TileMapFlag::NO_FLAGS);
    stateFlags = std::exchange(other.stateFlags, TileStateFlag::NO_FLAGS);
    itemTypeProvider = std::exchange(other.itemTypeProvider, nullptr);
    m_waypointCount = std::exchange(other.m_waypointCount, 0);

    m_spawnRadius = std::exchange(other.m_spawnRadius, 0);                 // Added
    m_spawnCreatureList = std::move(other.m_spawnCreatureList);           // Added
    m_spawnIntervalSeconds = std::exchange(other.m_spawnIntervalSeconds, 0); // Added

    other.mapFlags = TileMapFlag::NO_FLAGS;
    other.stateFlags = TileStateFlag::NO_FLAGS;
    other.itemTypeProvider = nullptr;
    other.m_waypointCount = 0;
    return *this;
}


std::unique_ptr<Tile> Tile::deepCopy() const {
    // Constructor now takes itemTypeProvider
    auto newTile = std::make_unique<Tile>(this->position, this->itemTypeProvider);

    // Copy primitive members not handled by constructor or copyMembersTo if they differ from default
    newTile->m_houseId = this->m_houseId;
    newTile->m_isHouseExit = this->m_isHouseExit;
    newTile->m_isProtectionZone = this->m_isProtectionZone; // Added
    newTile->mapFlags = this->mapFlags;
    newTile->stateFlags = this->stateFlags; // This might need selective copying, e.g. not MODIFIED
    // newTile->m_spawnDataRef = this->m_spawnDataRef; // Removed old spawn ref
    newTile->m_waypointCount = this->m_waypointCount;

    newTile->m_spawnRadius = this->m_spawnRadius;                 // Added
    newTile->m_spawnCreatureList = this->m_spawnCreatureList;     // Added
    newTile->m_spawnIntervalSeconds = this->m_spawnIntervalSeconds; // Added

    // Call the helper for unique_ptr members (ground, items, creature)
    // copyMembersTo(*newTile) was the old way. Now direct deep copy in copy constructor.
    // Let's keep copyMembersTo for explicit deepCopy method if needed,
    // but copy constructor/assignment should handle their own deep copies.
    // For deepCopy() method itself:
    if (this->ground) newTile->ground = this->ground->deepCopy();
    newTile->items.clear();
    for (const auto& item : this->items) {
        if (item) newTile->items.append(item->deepCopy());
    }
    if (this->creature) newTile->creature = this->creature->deepCopy();
    // No spawn unique_ptr to copy.

    return newTile;
}

// copyMembersTo is now primarily for deep-copying unique_ptr members if used by old deepCopy logic.
// However, with explicit deep copies in copy constructor/assignment, its role might diminish.
// For now, keep it for unique_ptrs if some external deep copy mechanism might use it.
void Tile::copyMembersTo(Tile& target) const {
    // Primitives and non-owning pointers are copied by Tile's own copy constructor/assignment
    // This helper focuses on heap-allocated members owned by unique_ptr.
    if (this->ground) {
        target.ground = this->ground->deepCopy();
    } else {
        target.ground.reset();
    }
    target.items.clear(); // Ensure target items list is empty before adding copies
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
    // if (this->spawn) { // Removed old spawn
    //     target.spawn = this->spawn->deepCopy();
    // } else {
    //     target.spawn.reset();
    // }

    // New spawn members are value types, copied by direct assignment in op= or copy ctor.
    target.m_spawnRadius = this->m_spawnRadius;
    target.m_spawnCreatureList = this->m_spawnCreatureList;
    target.m_spawnIntervalSeconds = this->m_spawnIntervalSeconds;
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

// New Spawn Data Methods Implementation
bool Tile::isSpawnTile() const {
    return m_spawnRadius > 0;
}

int Tile::getSpawnRadius() const {
    return m_spawnRadius;
}

void Tile::setSpawnRadius(int radius) {
    int newRadius = (radius >= 0 ? radius : 0);
    if (m_spawnRadius != newRadius) {
        m_spawnRadius = newRadius;
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

const QStringList& Tile::getSpawnCreatureList() const {
    return m_spawnCreatureList;
}

void Tile::setSpawnCreatureList(const QStringList& creatureList) {
    if (m_spawnCreatureList != creatureList) {
        m_spawnCreatureList = creatureList;
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

void Tile::addCreatureToSpawnList(const QString& creatureName) {
    if (!creatureName.isEmpty() && !m_spawnCreatureList.contains(creatureName)) {
        m_spawnCreatureList.append(creatureName);
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

bool Tile::removeCreatureFromSpawnList(const QString& creatureName) {
    if (m_spawnCreatureList.removeOne(creatureName)) {
        addStateFlag(TileStateFlag::MODIFIED);
        return true;
    }
    return false;
}

void Tile::clearSpawnCreatureList() {
    if (!m_spawnCreatureList.isEmpty()) {
        m_spawnCreatureList.clear();
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

int Tile::getSpawnIntervalSeconds() const {
    return m_spawnIntervalSeconds;
}

void Tile::setSpawnIntervalSeconds(int seconds) {
    int newInterval = (seconds >= 0 ? seconds : 0);
    if (m_spawnIntervalSeconds != newInterval) {
        m_spawnIntervalSeconds = newInterval;
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

void Tile::clearSpawnData() {
    bool changed = (m_spawnRadius != 0 || !m_spawnCreatureList.isEmpty() || m_spawnIntervalSeconds != 0);
    m_spawnRadius = 0;
    m_spawnCreatureList.clear();
    m_spawnIntervalSeconds = 0;
    if (changed) {
        addStateFlag(TileStateFlag::MODIFIED);
    }
}

// Updated isEmpty (implementation for declaration in .h)
bool Tile::isEmpty() const {
    return getItemCount() == 0 && !m_creature && !isSpawnTile();
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
        newBlockingState = true;
    }
    // Note: isSpawnTile() does not contribute to blocking status typically.

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
        memory += sizeof(RME::core::creatures::Creature) + 128; // Approx
    }
    // New spawn members:
    memory += sizeof(m_spawnRadius);
    memory += sizeof(m_spawnIntervalSeconds);
    for (const QString& name : m_spawnCreatureList) {
        memory += name.capacity() * sizeof(QChar); // Approximate string data
    }
    memory += m_spawnCreatureList.capacity() * sizeof(QString); // QList overhead

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
void Tile::setIsProtectionZone(bool isPZ) {
    if (m_isProtectionZone != isPZ) {
        m_isProtectionZone = isPZ;
        // If this direct flag should also set the TileMapFlag (and vice-versa), that logic would go here.
        // For now, they are separate. If TileMapFlag::PROTECTION_ZONE implies m_isProtectionZone,
        // then this setter might also call addMapFlag or removeMapFlag.
        // And isProtectionZone() might check both this flag AND the mapFlag.
        // For now, keeping them distinct as per simple member addition.
    }
}

bool Tile::isProtectionZone() const {
    // This returns the state of the direct boolean member.
    // The existing isPZ() returns based on TileMapFlag.
    // Depending on design, these might need to be reconciled or one might be preferred.
    return m_isProtectionZone;
}

} // namespace RME
