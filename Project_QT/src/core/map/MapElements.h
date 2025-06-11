#ifndef RME_MAP_ELEMENTS_H
#define RME_MAP_ELEMENTS_H

#include "../Position.h" // For RME::Position. Changed to relative path assuming Position.h is in core/
#include <QString>
#include <QList> // For list of creature names in SpawnData (though SpawnData itself is deferred)
#include <QRect> // For HouseData exit scrollbars/area (commented out for now)
// QSet include might be removed if WaypointData was the only user.
// #include <QSet>

namespace RME {

// --- TownData ---
struct TownData {
    quint32 id = 0;
    QString name;
    Position templePosition; // Central position or temple location

    TownData() = default;
    TownData(quint32 townId, const QString& townName, const Position& townPos)
        : id(townId), name(townName), templePosition(townPos) {}
};

// --- HouseData ---
// Forward declaration if House class will be more complex later
// class House;

// Basic data for a house, more details (door positions, tile lists)
// would be part of a full House class/manager.
struct HouseData {
    quint32 houseId = 0;        // Unique ID of the house
    QString name;             // Name of the house
    Position entryPosition;   // Main entry point (tile in front of door, or door itself)
    quint32 townId = 0;         // Town this house belongs to
    quint32 size = 0;           // Number of tiles in the house
    quint32 rent = 0;           // Rent price
    // QRect exitScrollbars;    // Original RME had this for house exit editor (commented for now)

    // More complex data like list of door IDs, tile coordinates, owner, etc.
    // will be handled by a dedicated House system/class later.

    HouseData() = default;
};


// --- SpawnData ---
// As per task description:
// For now, Map will interact with RME::Spawn objects on Tiles (from "core/Spawn.h").
// A specific MapElements::SpawnData struct or a list like MapSpawnLocation is deferred
// until a clear need arises for the Map class to store spawn information separately
// from the Tile-based RME::Spawn instances.

// --- WaypointData --- (This section is now removed)

// --- Other potential map-wide elements ---
// E.g., Map Signs, if they are stored globally by Map
/*
struct MapSignData {
    Position position;
    QString text;
};
*/

} // namespace RME

#endif // RME_MAP_ELEMENTS_H
