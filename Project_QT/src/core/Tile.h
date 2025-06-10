#ifndef RME_TILE_H
#define RME_TILE_H

#include "Position.h"
#include "Item.h"     // For std::unique_ptr<Item> and IItemTypeProvider for addItem logic
#include "Creature.h" // For std::unique_ptr<Creature>
#include "Spawn.h"    // For std::unique_ptr<Spawn>

#include <QList>
#include <memory> // For std::unique_ptr
#include <vector> // For returning collections of items
#include <QFlags> // Required for Q_DECLARE_FLAGS

namespace RME {

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

    // Deep copy
    std::unique_ptr<Tile> deepCopy() const;

    // Position
    const Position& getPosition() const { return position; }

    // Item Management
    // addItem returns raw pointer to item for convenience, but ownership remains with Tile.
    Item* addItem(std::unique_ptr<Item> item);
    void removeItem(Item* itemToRemove); // Removes and deletes the item
    std::unique_ptr<Item> popItem(Item* itemToPop); // Removes and returns ownership

    Item* getGround() const { return ground.get(); }
    const QList<std::unique_ptr<Item>>& getItems() const { return items; }
    QList<Item*> getAllItems() const; // Returns raw pointers to all items (ground + top)

    Item* getTopItem() const; // Gets the highest visible non-creature item
    Item* getItemAtStackpos(int stackpos) const; // 0 = ground, 1 = first top item, etc.
    int getItemCount() const { return (ground ? 1 : 0) + items.size(); }

    // Creature Management
    Creature* getCreature() const { return creature.get(); }
    void setCreature(std::unique_ptr<Creature> newCreature);
    std::unique_ptr<Creature> popCreature();

    // Spawn Management
    Spawn* getSpawn() const { return spawn.get(); }
    void setSpawn(std::unique_ptr<Spawn> newSpawn);
    std::unique_ptr<Spawn> popSpawn();

    // House ID
    uint32_t getHouseID() const { return house_id; }
    void setHouseID(uint32_t id) { house_id = id; }

    // Flags
    TileMapFlags getMapFlags() const { return mapFlags; }
    void setMapFlags(TileMapFlags flags) { mapFlags = flags; }
    void addMapFlag(TileMapFlag flag) { mapFlags |= flag; }
    void removeMapFlag(TileMapFlag flag) { mapFlags &= ~flag; }
    bool hasMapFlag(TileMapFlag flag) const { return mapFlags.testFlag(flag); }

    TileStateFlags getStateFlags() const { return stateFlags; }
    void setStateFlags(TileStateFlags flags) { stateFlags = flags; } // Usually for internal update
    void addStateFlag(TileStateFlag flag) { stateFlags |= flag; }
    void removeStateFlag(TileStateFlag flag) { stateFlags &= ~flag; }
    bool hasStateFlag(TileStateFlag flag) const { return stateFlags.testFlag(flag); }

    // Properties derived from items/flags
    bool isBlocking() const; // Checks items and tile state
    bool isPZ() const { return hasMapFlag(TileMapFlag::PROTECTION_ZONE); }
    // Add more like isNoPVP, isNoLogout, etc.

    // Update internal state (e.g., blocking flag) based on items
    void update();

    // Stubs for complex editor functions (to be implemented in later tasks)
    void borderize(const Tile* neighbors[8]) { /* TODO */ }
    void wallize() { /* TODO */ }
    void tableize() { /* TODO */ }
    void carpetize() { /* TODO */ }

    // Waypoint Tracking
    void increaseWaypointCount();
    void decreaseWaypointCount();
    int getWaypointCount() const;

    // Needed for addItem logic
    IItemTypeProvider* getItemTypeProvider() const { return itemTypeProvider; }


private:
    Position position;
    std::unique_ptr<Item> ground;
    QList<std::unique_ptr<Item>> items; // Items on top of ground
    std::unique_ptr<Creature> creature;
    std::unique_ptr<Spawn> spawn;
    uint32_t house_id = 0;

    TileMapFlags mapFlags = TileMapFlag::NO_FLAGS;
    TileStateFlags stateFlags = TileStateFlag::NO_FLAGS;

    IItemTypeProvider* itemTypeProvider; // Non-owning, passed at construction for item logic
    int m_waypointCount = 0;

    // Helper for deep copy
    void copyMembersTo(Tile& target) const;
};

} // namespace RME

#endif // RME_TILE_H
