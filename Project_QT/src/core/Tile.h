#ifndef RME_TILE_H
#define RME_TILE_H

#include "Position.h"
#include "Item.h"     // For std::unique_ptr<Item> and IItemTypeProvider for addItem logic
#include "core/creatures/Creature.h" // For std::unique_ptr<Creature>
#include "Spawn.h"    // For std::unique_ptr<Spawn>

#include <QList>
#include <memory> // For std::unique_ptr
#include <vector> // For returning collections of items
#include <QFlags> // Required for Q_DECLARE_FLAGS

namespace RME {

class SpawnData; // Forward declaration

// Tile flags matching original RME concepts, potentially with some Qt-ification
// These might be a combination of original mapflags and statflags
enum class TileMapFlag : uint32_t {
    NO_FLAGS            = 0,
    PROTECTION_ZONE     = 1 << 0, // TILESTATE_PROTECTIONZONE
    NO_PVP_ZONE         = 1 << 1, // TILESTATE_NOPVPZONE
    NO_LOGOUT_ZONE      = 1 << 2, // TILESTATE_NOLOGOUT
    PVP_ZONE            = 1 << 3, // TILESTATE_PVPZONE
    REFRESH             = 1 << 4, // TILESTATE_REFRESH, forces redraw or update
    // Add others from RME's tile.h (TILESTATE_*) that are persistent map flags
};
Q_DECLARE_FLAGS(TileMapFlags, TileMapFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TileMapFlags)

enum class TileStateFlag : uint32_t { // Internal/transient states
    NO_FLAGS            = 0,
    SELECTED            = 1 << 0,  // TILESTATE_SELECTED
    BLOCKING            = 1 << 1,  // TILESTATE_BLOCKING (cached from items)
    HAS_TABLE           = 1 << 2,  // TILESTATE_HAS_TABLE (cached)
    MODIFIED            = 1 << 3,  // TILESTATE_MODIFIED (dirty flag)
    HAS_WALKABLE_GROUND = 1 << 4,  // Custom: if ground item is walkable
    // Add others as needed
};
Q_DECLARE_FLAGS(TileStateFlags, TileStateFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(TileStateFlags)


class Tile {
public:
    Tile(const Position& pos, IItemTypeProvider* itemProvider); // itemProvider needed for addItem logic
    Tile(int x, int y, int z, IItemTypeProvider* itemProvider);
    ~Tile() = default; // std::unique_ptr will handle cleanup

    // Copying and Moving
    Tile(const Tile& other);
    Tile& operator=(const Tile& other);
    Tile(Tile&& other) noexcept;
    Tile& operator=(Tile&& other) noexcept;

    std::unique_ptr<Tile> deepCopy() const;

    // Position
    const Position& getPosition() const { return position; }

    // Item Management
    Item* addItem(std::unique_ptr<Item> item);
    void removeItem(Item* itemToRemove);
    std::unique_ptr<Item> popItem(Item* itemToPop);

    Item* getGround() const { return ground.get(); }
    const QList<std::unique_ptr<Item>>& getItems() const { return items; }
    QList<Item*> getAllItems() const;

    Item* getTopItem() const;
    Item* getItemAtStackpos(int stackpos) const;
    int getItemCount() const { return (ground ? 1 : 0) + items.size(); }
    bool isEmpty() const { return getItemCount() == 0 && !creature && !spawn; } // Updated isEmpty
    bool hasItemOfType(uint16_t id) const; // Example utility
    Item* getItemById(uint16_t id) const; // Example utility

    /**
     * @brief Sets the ground item for this tile.
     * Takes ownership of the newGround item. Replaces any existing ground item.
     * If newGround is nullptr, the ground item is cleared.
     * @param newGround A unique_ptr to the new ground item, or nullptr to clear.
     */
    void setGround(std::unique_ptr<Item> newGround);

    /**
     * @brief Removes the first item (ground or stacked) matching the given ID.
     * If a ground item matches, it's removed. Otherwise, searches stacked items.
     * @param itemId The ID of the item to remove.
     * @return True if an item was found and removed, false otherwise.
     */
    bool removeItemById(uint16_t itemId);

    // Creature Management
    const RME::core::creatures::Creature* getCreature() const { return creature.get(); }
    RME::core::creatures::Creature* getCreature() { return creature.get(); } // Non-const version
    void setCreature(std::unique_ptr<RME::core::creatures::Creature> newCreature);
    std::unique_ptr<RME::core::creatures::Creature> popCreature();
    bool hasCreature() const { return creature != nullptr; }

    // Spawn Management
    Spawn* getSpawn() const { return spawn.get(); }
    void setSpawn(std::unique_ptr<Spawn> newSpawn);
    std::unique_ptr<Spawn> popSpawn();
    bool hasSpawn() const { return spawn != nullptr; }

    // SpawnData Reference (if this tile is a spawn center)
    RME::SpawnData* getSpawnDataRef() const { return m_spawnDataRef; }
    void setSpawnDataRef(RME::SpawnData* ref) { m_spawnDataRef = ref; }

    // House ID
    uint32_t getHouseId() const { return m_houseId; }
    void setHouseId(uint32_t id) { m_houseId = id; }

    bool isHouseExit() const;
    void setIsHouseExit(bool isExit);

    void setIsProtectionZone(bool isPZ); // Added
    bool isProtectionZone() const;       // Added

    // Flags
    TileMapFlags getMapFlags() const { return mapFlags; }
    void setMapFlags(TileMapFlags flags) { mapFlags = flags; }
    void addMapFlag(TileMapFlag flag) { mapFlags |= flag; }
    void removeMapFlag(TileMapFlag flag) { mapFlags &= ~flag; }
    bool hasMapFlag(TileMapFlag flag) const { return mapFlags.testFlag(flag); }

    TileStateFlags getStateFlags() const { return stateFlags; }
    void setStateFlags(TileStateFlags flags) { stateFlags = flags; }
    void addStateFlag(TileStateFlag flag) { stateFlags |= flag; }
    void removeStateFlag(TileStateFlag flag) { stateFlags &= ~flag; }
    bool hasStateFlag(TileStateFlag flag) const { return stateFlags.testFlag(flag); }

    // Properties derived from items/flags
    bool isBlocking() const;
    bool isPZ() const { return hasMapFlag(TileMapFlag::PROTECTION_ZONE); }

    void update();

    void borderize(const Tile* neighbors[8]) { /* TODO */ }
    void wallize() { /* TODO */ }
    void tableize() { /* TODO */ }
    void carpetize() { /* TODO */ }

    void increaseWaypointCount();
    void decreaseWaypointCount();
    int getWaypointCount() const;

    IItemTypeProvider* getItemTypeProvider() const { return itemTypeProvider; }
    size_t estimateMemoryUsage() const;

private:
    Position position;
    std::unique_ptr<Item> ground;
    QList<std::unique_ptr<Item>> items;
    std::unique_ptr<RME::core::creatures::Creature> creature;
    std::unique_ptr<Spawn> spawn;
    RME::SpawnData* m_spawnDataRef = nullptr; // Non-owning pointer to a spawn centered here
    uint32_t m_houseId = 0; // Renamed from house_id
    bool m_isHouseExit = false; // New flag

    TileMapFlags mapFlags = TileMapFlag::NO_FLAGS;
    TileStateFlags stateFlags = TileStateFlag::NO_FLAGS;

    IItemTypeProvider* itemTypeProvider;
    int m_waypointCount = 0;
    bool m_isProtectionZone = false; // Added

    void copyMembersTo(Tile& target) const;
};

} // namespace RME

#endif // RME_TILE_H
