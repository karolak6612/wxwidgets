#include "Tile.h"
#include "core/spawns/Spawn.h"
#include "core/assets/MaterialManager.h"
#include "core/assets/MaterialData.h"
#include "core/map/Map.h"
#include <algorithm> // For std::remove_if
#include <QDebug>    // For qWarning() logging
#include <cstdint>   // For uint8_t, uint16_t

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
        qWarning() << "Tile created with null IItemTypeProvider at position" 
                   << pos.x << "," << pos.y << "," << pos.z;
        // Note: We allow null provider but warn about it for debugging
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
        qWarning() << "Tile created with null IItemTypeProvider at position" 
                   << x << "," << y << "," << z;
        // Note: We allow null provider but warn about it for debugging
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
    if (!item) {
        qWarning() << "Tile::addItem: Attempted to add null item at position" 
                   << position.x << "," << position.y << "," << position.z;
        return nullptr;
    }
    if (!itemTypeProvider) {
        qWarning() << "Tile::addItem: No itemTypeProvider available at position" 
                   << position.x << "," << position.y << "," << position.z;
        return nullptr;
    }
    
    Item* rawItemPtr = item.get();
    if (itemTypeProvider->isGround(item->getID())) {
        if (ground) {
            qDebug() << "Tile::addItem: Replacing existing ground item" << ground->getID() 
                     << "with" << item->getID() << "at position" 
                     << position.x << "," << position.y << "," << position.z;
        }
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

void Tile::clearSpawn() {
    m_spawnRadius = 0;
    m_spawnIntervalSeconds = 0;
    m_spawnCreatureList.clear();
    addStateFlag(TileStateFlag::MODIFIED);
}

void Tile::clearSpawnData() { // Legacy compatibility
    clearSpawn(); // Delegate to new method
}

// Updated isEmpty (implementation for declaration in .h)
bool Tile::isEmpty() const {
    return getItemCount() == 0 && !creature && !isSpawnTile();
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
        // Mark tile as modified for live map integration
        addStateFlag(TileStateFlag::MODIFIED);
        
        // In a full implementation with live map support, this would also:
        // - Notify the map that this tile needs to be marked dirty for rendering
        // - Trigger network updates for live collaboration
        // - Update any cached house data structures
    }
}

bool Tile::isHouseExit() const {
    return m_isHouseExit;
}

void Tile::setIsHouseExit(bool isExit) {
    if (m_isHouseExit != isExit) {
        m_isHouseExit = isExit;
        // Mark tile as modified for live map integration
        addStateFlag(TileStateFlag::MODIFIED);
        
        // In a full implementation with live map support, this would also:
        // - Notify the map that this tile needs to be marked dirty for rendering
        // - Trigger network updates for live collaboration
        // - Update any cached house exit data structures
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

// --- Spawn Integration ---
void Tile::setSpawn(const RME::core::spawns::Spawn& spawn) {
    m_spawnRadius = spawn.getRadius();
    m_spawnCreatureList = spawn.getCreatureTypes();
    m_spawnIntervalSeconds = spawn.getIntervalSeconds();
    addStateFlag(TileStateFlag::MODIFIED);
}

RME::core::spawns::Spawn Tile::getSpawn() const {
    Position tilePos(position.x(), position.y(), position.z());
    RME::core::spawns::Spawn spawn(tilePos, m_spawnRadius, m_spawnIntervalSeconds);
    spawn.setCreatureTypes(m_spawnCreatureList);
    return spawn;
}

bool Tile::hasSpawn() const {
    return m_spawnRadius > 0 || !m_spawnCreatureList.isEmpty() || m_spawnIntervalSeconds > 0;
}

bool Tile::hasSpawnData() const {
    return hasSpawn(); // Legacy compatibility
}

// --- Tile Operations Implementation ---

namespace {
    // Forward declarations for helper functions
    bool hasCompatibleMaterial(const Tile* tile, const QString& materialType);
    QString neighborConfigToEdgeString(uint8_t config);
    
    // Helper function to analyze neighbors for border operations
    uint8_t analyzeNeighbors(const Tile* neighbors[8], const QString& materialType) {
        uint8_t config = 0;
        // Check 8 neighbors in order: N, NE, E, SE, S, SW, W, NW
        for (int i = 0; i < 8; ++i) {
            if (neighbors[i] && hasCompatibleMaterial(neighbors[i], materialType)) {
                config |= (1 << i);
            }
        }
        return config;
    }
    
    // Helper function to check if a tile has compatible material
    bool hasCompatibleMaterial(const Tile* tile, const QString& materialType) {
        if (!tile || !tile->getGround()) return false;
        
        // This is a simplified check - in a full implementation, we'd check
        // the material database to see if the ground item belongs to the specified material
        // For now, we'll use a basic item ID range check
        uint16_t itemId = tile->getGround()->getID();
        
        // Example material ranges (these would come from MaterialManager in practice)
        if (materialType == "grass") {
            return (itemId >= 100 && itemId <= 120); // Example grass range
        } else if (materialType == "stone") {
            return (itemId >= 200 && itemId <= 220); // Example stone range
        } else if (materialType == "water") {
            return (itemId >= 300 && itemId <= 320); // Example water range
        }
        
        return false;
    }
    
    // Helper function to get border item ID based on configuration
    uint16_t getBorderItemId(uint8_t config, const QString& borderSetId, const RME::core::assets::MaterialManager* materialManager) {
        if (!materialManager) return 0;
        
        const auto* borderSet = materialManager->getBorderSet(borderSetId);
        if (!borderSet) return 0;
        
        // Convert 8-bit neighbor configuration to border edge string
        QString edgeString = neighborConfigToEdgeString(config);
        
        auto it = borderSet->edgeItems.find(edgeString);
        if (it != borderSet->edgeItems.end()) {
            return it.value();
        }
        
        return 0;
    }
    
    // Helper function to convert neighbor configuration to edge string
    QString neighborConfigToEdgeString(uint8_t config) {
        // This is a simplified mapping - the actual implementation would be more complex
        // and based on the specific border system used by RME
        
        // Check for common patterns
        if (config == 0) return "center"; // No neighbors
        if ((config & 0x11) == 0x11) return "n"; // North neighbors
        if ((config & 0x44) == 0x44) return "s"; // South neighbors
        if ((config & 0x22) == 0x22) return "e"; // East neighbors
        if ((config & 0x88) == 0x88) return "w"; // West neighbors
        
        // Corner patterns
        if ((config & 0x13) == 0x13) return "ne"; // Northeast
        if ((config & 0x31) == 0x31) return "se"; // Southeast
        if ((config & 0x62) == 0x62) return "sw"; // Southwest
        if ((config & 0xC4) == 0xC4) return "nw"; // Northwest
        
        return "center"; // Default fallback
    }
    
    // Helper function to get wall segment type based on 4-neighbor configuration
    uint32_t getWallSegmentType(uint8_t config4) {
        // Wall types lookup table for 4-neighbor configuration
        // This maps to BorderType enum values from BrushEnums.h
        static const uint32_t wallTypes[16] = {
            0,  // 0000 - no walls
            1,  // 0001 - wall to north
            2,  // 0010 - wall to east
            3,  // 0011 - corner NE
            4,  // 0100 - wall to south
            5,  // 0101 - vertical wall N-S
            6,  // 0110 - corner SE
            7,  // 0111 - T-junction E
            8,  // 1000 - wall to west
            9,  // 1001 - corner NW
            10, // 1010 - horizontal wall E-W
            11, // 1011 - T-junction N
            12, // 1100 - corner SW
            13, // 1101 - T-junction W
            14, // 1110 - T-junction S
            15  // 1111 - cross junction
        };
        
        return wallTypes[config4 & 0x0F];
    }
    
    // Helper function to get carpet/table part alignment based on 8-neighbor configuration
    QString getCarpetAlignment(uint8_t config) {
        // Simplified carpet alignment logic
        // In practice, this would be more sophisticated
        
        bool hasN = (config & 0x01) != 0;
        bool hasE = (config & 0x04) != 0;
        bool hasS = (config & 0x10) != 0;
        bool hasW = (config & 0x40) != 0;
        
        // Edge pieces
        if (!hasN && hasE && hasS && hasW) return "n";
        if (hasN && !hasE && hasS && hasW) return "e";
        if (hasN && hasE && !hasS && hasW) return "s";
        if (hasN && hasE && hasS && !hasW) return "w";
        
        // Corner pieces
        if (!hasN && !hasE && hasS && hasW) return "ne";
        if (!hasN && hasE && !hasS && hasW) return "se";
        if (hasN && hasE && !hasS && !hasW) return "sw";
        if (hasN && !hasE && hasS && !hasW) return "nw";
        
        // Center piece
        if (hasN && hasE && hasS && hasW) return "center";
        
        return "center"; // Default
    }
}

void Tile::borderize(const Tile* neighbors[8]) {
    if (!itemTypeProvider) {
        qWarning() << "Tile::borderize: No itemTypeProvider available";
        return;
    }
    
    // Get the current ground material type
    if (!ground) {
        return; // No ground to borderize
    }
    
    // For now, we'll use a simplified approach
    // In a full implementation, we'd get the MaterialManager from a global context
    // and determine the material type from the ground item
    
    QString materialType = "grass"; // Simplified - would be determined from ground item
    uint8_t config = analyzeNeighbors(neighbors, materialType);
    
    // Apply borders based on neighbor configuration
    // This is a simplified implementation - the full version would:
    // 1. Get MaterialManager from global context
    // 2. Determine material type from ground item
    // 3. Get border rules from material data
    // 4. Apply appropriate border items
    
    // For now, just mark the tile as modified
    addStateFlag(TileStateFlag::MODIFIED);
    
    qDebug() << "Tile::borderize applied at position" << position.x << "," << position.y 
             << "with neighbor config" << QString::number(config, 16);
}

void Tile::wallize() {
    if (!itemTypeProvider) {
        qWarning() << "Tile::wallize: No itemTypeProvider available";
        return;
    }
    
    // Wallization logic would:
    // 1. Analyze 4-directional neighbors for wall connectivity
    // 2. Determine appropriate wall segment type (horizontal, vertical, corner, junction)
    // 3. Replace current wall items with correct orientation
    
    // For now, implement basic logic
    // In practice, this would need access to the map to check neighbors
    
    // Check if this tile has wall items
    // For now, use a simplified check based on item ID ranges
    // In a full implementation, this would check the MaterialManager
    bool hasWallItems = false;
    for (const auto& item : items) {
        if (item) {
            uint16_t itemId = item->getID();
            // Example wall item ID range (would come from MaterialManager in practice)
            if (itemId >= 1000 && itemId <= 1200) {
                hasWallItems = true;
                break;
            }
        }
    }
    
    if (!hasWallItems) {
        return; // No walls to wallize
    }
    
    // Mark as modified for now
    addStateFlag(TileStateFlag::MODIFIED);
    
    qDebug() << "Tile::wallize applied at position" << position.x << "," << position.y;
}

void Tile::tableize() {
    if (!itemTypeProvider) {
        qWarning() << "Tile::tableize: No itemTypeProvider available";
        return;
    }
    
    // Tableization logic would:
    // 1. Analyze 8-directional neighbors for table connectivity
    // 2. Determine appropriate table part (center, edge, corner)
    // 3. Replace current table items with correct parts
    
    // Check if this tile has table items
    // For now, use a simplified check based on item ID ranges
    bool hasTableItems = false;
    for (const auto& item : items) {
        if (item) {
            uint16_t itemId = item->getID();
            // Example table item ID range (would come from MaterialManager in practice)
            if (itemId >= 1500 && itemId <= 1600) {
                hasTableItems = true;
                break;
            }
        }
    }
    
    if (!hasTableItems) {
        return; // No tables to tableize
    }
    
    // Set the HAS_TABLE state flag
    addStateFlag(TileStateFlag::HAS_TABLE);
    addStateFlag(TileStateFlag::MODIFIED);
    
    qDebug() << "Tile::tableize applied at position" << position.x << "," << position.y;
}

void Tile::carpetize() {
    if (!itemTypeProvider) {
        qWarning() << "Tile::carpetize: No itemTypeProvider available";
        return;
    }
    
    // Carpetization logic would:
    // 1. Analyze 8-directional neighbors for carpet connectivity
    // 2. Determine appropriate carpet part (center, edge, corner)
    // 3. Replace current carpet items with correct parts
    
    // Check if this tile has carpet items
    // For now, use a simplified check based on item ID ranges
    bool hasCarpetItems = false;
    for (const auto& item : items) {
        if (item) {
            uint16_t itemId = item->getID();
            // Example carpet item ID range (would come from MaterialManager in practice)
            if (itemId >= 1300 && itemId <= 1400) {
                hasCarpetItems = true;
                break;
            }
        }
    }
    
    if (!hasCarpetItems) {
        return; // No carpets to carpetize
    }
    
    // Mark as modified
    addStateFlag(TileStateFlag::MODIFIED);
    
    qDebug() << "Tile::carpetize applied at position" << position.x << "," << position.y;
}

// Advanced tile operations
void Tile::optimizeItemStack() {
    if (items.isEmpty()) {
        return;
    }
    
    // Remove null items
    items.removeAll(nullptr);
    
    // Sort items by their stacking order (if needed)
    // This is a simplified optimization - in practice, we'd sort by:
    // 1. Always-on-top items first
    // 2. Regular items by their natural stacking order
    // 3. Ground items last (though ground is separate)
    
    // For now, just ensure no null pointers and mark as modified
    addStateFlag(TileStateFlag::MODIFIED);
    
    qDebug() << "Tile::optimizeItemStack optimized" << items.size() << "items at position" 
             << position.x << "," << position.y;
}

void Tile::validateTileState() {
    bool stateChanged = false;
    
    // Validate that state flags match actual tile content
    bool shouldHaveTable = false;
    bool shouldBeBlocking = false;
    bool shouldHaveWalkableGround = false;
    
    if (itemTypeProvider) {
        // Check ground properties
        if (ground) {
            if (itemTypeProvider->isBlocking(ground->getID())) {
                shouldBeBlocking = true;
            }
            if (itemTypeProvider->isWalkable(ground->getID())) {
                shouldHaveWalkableGround = true;
            }
        }
        
        // Check item properties
        for (const auto& item : items) {
            if (item) {
                uint16_t itemId = item->getID();
                
                if (itemTypeProvider->isBlocking(itemId)) {
                    shouldBeBlocking = true;
                }
                
                // Check for table items (simplified)
                if (itemId >= 1500 && itemId <= 1600) {
                    shouldHaveTable = true;
                }
            }
        }
    }
    
    // Check creature blocking
    if (creature) {
        shouldBeBlocking = true;
    }
    
    // Update state flags if they don't match reality
    if (shouldBeBlocking != hasStateFlag(TileStateFlag::BLOCKING)) {
        if (shouldBeBlocking) {
            addStateFlag(TileStateFlag::BLOCKING);
        } else {
            removeStateFlag(TileStateFlag::BLOCKING);
        }
        stateChanged = true;
    }
    
    if (shouldHaveTable != hasStateFlag(TileStateFlag::HAS_TABLE)) {
        if (shouldHaveTable) {
            addStateFlag(TileStateFlag::HAS_TABLE);
        } else {
            removeStateFlag(TileStateFlag::HAS_TABLE);
        }
        stateChanged = true;
    }
    
    if (shouldHaveWalkableGround != hasStateFlag(TileStateFlag::HAS_WALKABLE_GROUND)) {
        if (shouldHaveWalkableGround) {
            addStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);
        } else {
            removeStateFlag(TileStateFlag::HAS_WALKABLE_GROUND);
        }
        stateChanged = true;
    }
    
    if (stateChanged) {
        addStateFlag(TileStateFlag::MODIFIED);
        qDebug() << "Tile::validateTileState corrected state flags at position" 
                 << position.x << "," << position.y;
    }
}

bool Tile::needsUpdate() const {
    return hasStateFlag(TileStateFlag::MODIFIED);
}

void Tile::markDirty() {
    addStateFlag(TileStateFlag::MODIFIED);
    
    // In a full implementation with live map support, this would also:
    // - Add this tile to the map's dirty tile list
    // - Schedule a rendering update
    // - Trigger network synchronization for live collaboration
}

void Tile::clearDirty() {
    removeStateFlag(TileStateFlag::MODIFIED);
    
    // In a full implementation, this would be called after:
    // - The tile has been rendered
    // - The tile has been saved to disk
    // - Network synchronization is complete
}

} // namespace RME
